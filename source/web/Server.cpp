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
  ioContext(1), acceptor(ioContext) {
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
  thread  = new std::thread(&Server::run, this);
}

/**
 * @brief Execute thread operations
 * Accepts new connections, reads and writes open connections
 *
 */
void Server::run() {
  Net::socket_t *  socket = nullptr;
  Net::endpoint_t  endpoint;
  asio::error_code errorCode;
  bool             didSomething = false;

  auto now    = sysclk_t::now();
  timeoutTime = now + TIMEOUT_NO_CONNECTIONS;

  while (running) {
    didSomething = false;
    now          = sysclk_t::now();

    // Check for new connections
    if (socket == nullptr)
      socket = new Net::socket_t(ioContext);

    acceptor.accept(*socket, endpoint, errorCode);
    if (!errorCode) {
      std::string endpointString = endpointStr(endpoint);
      info("Opening connection to " + endpointString);
      connections.push_back(new Connection(socket, endpointString, now));
      socket       = nullptr;
      didSomething = true;
    } else if (errorCode != asio::error::would_block) {
      running      = false;
      didSomething = true;
    }
    // else no waiting connections

    // Process current connections
    auto it = connections.begin();
    while (it != connections.end()) {
      Connection * connection = *it;

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
            log(EBLogLevel_t::EB_INFO, "Connection to %s closing",
                connection->toString().c_str());
            connection->stop();
            delete connection;
            it = connections.erase(it);
            break;
          default:
            warn("Unknown connection state");
        }
      } catch (const std::exception & e) {
        log(EBLogLevel_t::EB_WARNING,
            "Connection to %s closing due to exception: %s",
            connection->toString().c_str(), e.what());
        connection->stop();
        delete connection;
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
    delete socket;
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
  delete thread;
  thread = nullptr;
  for (Connection * connection : connections) {
    connection->stop();
    delete connection;
  }
  connections.clear();
}

/**
 * @brief Attach a callback to input originating from the uri
 *
 * @param uri of the source page to subscribe to
 * @param inputCallback function
 */
void Server::attachCallback(
    const std::string & uri, const EBInputCallback_t inputCallback) {
  inputCallbacks.emplace(uri, inputCallback);
}

/**
 * @brief Attach a callback to input files originating from the uri
 *
 * @param uri of the source page to subscribe to
 * @param inputCallback function
 */
void Server::attachCallback(
    const std::string & uri, const EBInputFileCallback_t inputCallback) {
  inputFileCallbacks.emplace(uri, inputCallback);
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
 * @return true when connections have been idle for greater than the timeout or
 * server is not running
 * @return false otherwise
 */
bool Server::isDone() const {
  return (sysclk_t::now() > timeoutTime && connections.empty()) || !running;
}

} // namespace Web
} // namespace Ehbanana