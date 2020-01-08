#include "Server.h"

#include "EhbananaLog.h"
#include "HTTP/CacheControl.h"
#include "HTTP/MIMETypes.h"

#include <string>

namespace Ehbanana {
namespace Web {

/**
 * @brief Construct a new Server:: Server object
 * Configure settings of the server, folder directories
 *
 * Opens the acceptor at the desired address and places it into listening mode
 *
 * Passing in PORT_AUTO for port will attempt to use the default port then
 * increment until an available port is open
 */
Server::Server(const EBGUISettings_t settings) :
  ioContext(1), acceptor(ioContext), pool(settings.callbackThreadPool) {
  TIMEOUT_NO_CONNECTIONS = seconds_t(settings.timeoutIdle);

  HTTP::HTTP::setRoot(settings.httpRoot);
  HTTP::CacheControl::Instance()->populateList(
      settings.configRoot + std::string("/cache.xml"));

  HTTP::MIMETypes::Instance()->populateList(
      settings.configRoot + std::string("/mime.types"));

  uint16_t port = settings.httpPort;

  Net::endpoint_t endpoint;
  try {
    Net::address address = Net::make_address(settings.ipAddress);
    endpoint             = Net::endpoint_t(address, port);
    acceptor.open(endpoint.protocol());
    acceptor.set_option(Net::acceptor_t::reuse_address(true));
  } catch (const asio::system_error & e) {
    throw std::exception(
        ("Creating acceptor: " + std::string(e.what())).c_str());
  }

  asio::error_code errorCode;
  // Attempt to bind until an available port is found
  bool attemptComplete = true;
  if (port == PORT_AUTO) {
    port            = PORT_DEFAULT;
    attemptComplete = false;
  }

  do {
    endpoint.port(port);
    acceptor.bind(endpoint, errorCode);
    if (!errorCode)
      attemptComplete = true;
    else if (errorCode.value() != asio::error::address_in_use &&
             errorCode.value() != asio::error::access_denied) {
      std::string message = "Binding acceptor to " + endpointStr(endpoint);
      message +=
          "\n#" + std::to_string(errorCode.value()) + " " + errorCode.message();
      throw std::exception(message.c_str());
    }
    // else address already in use, try the next one
    ++port;
  } while (!attemptComplete && port < 65535);

  try {
    acceptor.non_blocking(true);
    acceptor.listen(Net::socket_t::max_listen_connections);
  } catch (const asio::system_error & e) {
    throw std::exception(
        ("Setting acceptor options: " + std::string(e.what())).c_str());
  }

  domainName = endpointStr(endpoint);
}

/**
 * @brief Destroy the Server:: Server object
 * Safely stop the thread and any open connections
 */
Server::~Server() {
  stop();
}

/**
 * @brief Start the run thread
 *
 * @throw std::exception Thrown on failure
 */
void Server::start() {
  stop();

  running = true;
  thread  = std::make_unique<std::thread>(&Server::run, this);
}

/**
 * @brief Execute thread operations
 * Accepts new connections, reads and writes open connections
 *
 */
void Server::run() {
  std::unique_ptr<Net::socket_t> socket;
  Net::endpoint_t                endpoint;
  asio::error_code               errorCode;
  bool                           didSomething = false;

  auto now    = sysclk_t::now();
  timeoutTime = now + TIMEOUT_NO_CONNECTIONS;

  while (running) {
    didSomething = false;
    now          = sysclk_t::now();

    // Check for new connections
    if (socket == nullptr)
      socket = std::make_unique<Net::socket_t>(ioContext);

    acceptor.accept(*socket, endpoint, errorCode);
    if (!errorCode) {
      std::string endpointString = endpointStr(endpoint);
      info("Opening connection to " + endpointString);
      connections.push_back(std::make_unique<Connection>(
          std::move(socket), endpointString, now, this));
      socket       = nullptr;
      didSomething = true;
    } else if (errorCode != asio::error::would_block) {
      running      = false;
      didSomething = true;
    }
    // else no waiting connections

    std::shared_ptr<Ehbanana::MessageOut> messageOut = nullptr;
    if (!messagesOut.empty()) {
      messageOut = messagesOut.front();
      messagesOut.pop_front();
    }

    // Process current connections
    auto it = connections.begin();
    while (it != connections.end()) {
      const std::unique_ptr<Connection> & connection = *it;

      if (messageOut != nullptr)
        connection->enqueueOutput(messageOut);

      // Remove the connection and delete if update returns the connection is
      // complete
      try {
        connection->update(now);
        switch (connection->getState()) {
          case Connection::State_t::BUSY:
            didSomething = true;
          case Connection::State_t::IDLE:
            ++it;
            break;
          case Connection::State_t::DONE:
            log(EBLogLevel_t::EB_INFO, "%s: Connection closing",
                connection->toString().c_str());
            connection->stop();
            it = connections.erase(it);
            break;
          default:
            warn("Unknown connection state");
        }
      } catch (const std::exception & e) {
        log(EBLogLevel_t::EB_WARNING,
            "%s: Connection closing due to exception: %s",
            connection->toString().c_str(), e.what());
        connection->stop();
        it = connections.erase(it);
      }
    }

    if (connections.empty()) {
      if (timeoutTime == timepoint_t<sysclk_t>::min()) {
        timeoutTime = now + TIMEOUT_NO_CONNECTIONS;
      }
    } else {
      timeoutTime = timepoint_t<sysclk_t>::min();
    }

    // If nothing was processed this loop, sleep until the next to save CPU
    // usage
    if (!didSomething)
      std::this_thread::sleep_for(millis_t(1));
  }

  // Free socket
  if (socket != nullptr) {
    if (socket->is_open()) {
      socket->shutdown(Net::socket_t::shutdown_both);
      socket->close();
    }
  }
}

/**
 * @brief Stop the run thread
 *
 */
void Server::stop() {
  running = false;
  if (thread == nullptr)
    return;
  if (thread->joinable())
    thread->join();
  connections.clear();
  messagesOut.clear();
}

/**
 * @brief Attach a callback to input originating from the uri
 *
 * @param uri of the source page to subscribe to
 * @param callback function
 */
void Server::attachCallback(
    const std::string & uri, const EBInputCallback_t callback) {
  inputCallbacks.emplace(uri, callback);
}

/**
 * @brief Attach a callback to input files originating from the uri
 *
 * @param uri of the source page to subscribe to
 * @param callback function
 */
void Server::attachCallback(
    const std::string & uri, const EBInputFileCallback_t callback) {
  inputFileCallbacks.emplace(uri, callback);
}

/**
 * @brief Set the callback for output files not found in the HTTP tree
 * 404 errors calls this first
 *
 * @param callback function
 */
void Server::setOutputCallback(const EBOutputFileCallback_t callback) {
  outputFileCallback = callback;
}

/**
 * @brief Enqueue a callback routine in the thread pool
 *
 * @param uri of the triggering page
 * @param id of the triggering element
 * @param value of the triggering element
 */
void Server::enqueueCallback(const std::string & uri, const std::string & id,
    const std::string & value) {
  if (inputCallbacks.find(uri) != inputCallbacks.end()) {
    EBInputCallback_t func = inputCallbacks[uri];
    pool.push([func, id, value](
                  int /* threadID */) { (func)(id.c_str(), value.c_str()); });
  } else
    warn("No input callback found for \"" + uri + "\"");
}

/**
 * @brief Enqueue a callback routine in the thread pool
 *
 * @param uri of the triggering page
 * @param id of the triggering element
 * @param value of the element
 * @param stream of the triggering file
 */
void Server::enqueueCallback(const std::string & uri, const std::string & id,
    const std::string & value, std::shared_ptr<Stream> stream) {
  if (inputFileCallbacks.find(uri) != inputFileCallbacks.end()) {
    EBInputFileCallback_t func = inputFileCallbacks[uri];
    pool.push([func, id, value, stream](int /* threadID */) {
      (func)(id.c_str(), value.c_str(), (EBStream_t)stream.get());
    });
  } else
    warn("No input file callback found for \"" + uri + "\"");
}

/**
 * @brief Enqueue an output message
 *
 * @param message
 */
void Server::enqueueOutput(std::shared_ptr<Ehbanana::MessageOut> message) {
  messagesOut.emplace_back(message);
}

/**
 * @brief Get the domain name the server is listening to
 *
 * @return const std::string& domain name: '127.0.0.1:8080'
 */
const std::string & Server::getDomainName() const {
  return domainName;
}

/**
 * @brief Check if the server is complete
 *
 * @return true when connections have been idle for greater than the timeout
 * or server is not running
 * @return false otherwise
 */
bool Server::isDone() const {
  return (sysclk_t::now() > timeoutTime && connections.empty()) || !running;
}

} // namespace Web
} // namespace Ehbanana