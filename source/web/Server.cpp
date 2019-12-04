#include "Server.h"

#include "EhbananaLog.h"
#include "HTTP/CacheControl.h"
#include "HTTP/MIMETypes.h"

#include <string>

namespace Ehbanana {
namespace Web {

/**
 * @brief Construct a new Server:: Server object
 *
 */
Server::Server() : ioContext(1), acceptor(ioContext) {}

/**
 * @brief Destroy the Server:: Server object
 * Safely stop the thread and any open connections
 */
Server::~Server() {
  stop();
}

/**
 * @brief Initialize the server
 * Configure settings of the server, folder directories
 *
 * Opens the acceptor at the desired address and places it into listening mode
 *
 * Passing in PORT_AUTO for port will attempt to use the default port then
 * increment until an available port is open
 *
 * @param EBGUISettings_t settings
 * @return Result error code
 */
Result Server::initialize(const EBGUISettings_t settings) {
  Result result;

  TIMEOUT_NO_CONNECTIONS = std::chrono::seconds(settings.timeoutIdle);

  HTTP::HTTP::setRoot(settings.httpRoot);
  result = HTTP::CacheControl::Instance()->populateList(
      settings.configRoot + std::string("/cache.xml"));
  if (!result)
    return result + "Configuring server's cache control";

  result = HTTP::MIMETypes::Instance()->populateList(
      settings.configRoot + std::string("/mime.types"));
  if (!result)
    return result + "Configuring server's mime types";

  uint16_t port = settings.httpPort;

  asio::ip::tcp::endpoint endpoint;
  try {
    asio::ip::address address = asio::ip::make_address("127.0.0.1");
    endpoint                  = asio::ip::tcp::endpoint(address, port);
    acceptor.open(endpoint.protocol());
    acceptor.set_option(asio::ip::tcp::acceptor::reuse_address(true));
  } catch (const asio::system_error & e) {
    return ResultCode_t::EXCEPTION_OCCURRED + e.what() + "Creating acceptor";
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
             errorCode.value() != asio::error::access_denied)
      return ResultCode_t::BIND_FAILED +
             (errorCode.message() + " #" + std::to_string(errorCode.value())) +
             ("Server acceptor bind to " + endpoint.address().to_string() +
                 ":" + std::to_string(port));
    // else address already in use, try the next one
    ++port;
  } while (!attemptComplete && port < 65535);

  try {
    acceptor.non_blocking(true);
    acceptor.listen(asio::ip::tcp::socket::max_listen_connections);
  } catch (const asio::system_error & e) {
    return ResultCode_t::EXCEPTION_OCCURRED + e.what() +
           "Setting acceptor options";
  }

  domainName =
      endpoint.address().to_string() + ":" + std::to_string(endpoint.port());

  return ResultCode_t::SUCCESS;
}

/**
 * @brief Start the run thread
 *
 * @return Result
 */
Result Server::start() {
  if (domainName.empty())
    return ResultCode_t::INVALID_STATE + "Server not yet initialized";

  stop();

  running = true;
  thread  = new std::thread(&Server::run, this);

  return ResultCode_t::SUCCESS;
}

/**
 * @brief Execute thread operations
 * Accepts new connections, reads and writes open connections
 *
 */
void Server::run() {
  asio::ip::tcp::socket * socket = nullptr;
  asio::ip::tcp::endpoint endpoint;
  asio::error_code        errorCode;
  Result                  result;
  bool                    didSomething            = false;
  bool                    outputMessageDispatched = false;

  auto now    = std::chrono::system_clock::now();
  timeoutTime = now + TIMEOUT_NO_CONNECTIONS;

  while (running) {
    didSomething = false;
    now          = std::chrono::system_clock::now();

    // Check for new connections
    if (socket == nullptr)
      socket = new asio::ip::tcp::socket(ioContext);

    acceptor.accept(*socket, endpoint, errorCode);
    if (!errorCode) {
      std::string endpointString = endpoint.address().to_string() + ":" +
                                   std::to_string(endpoint.port());
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
    std::list<Connection *>::iterator i   = connections.begin();
    std::list<Connection *>::iterator end = connections.end();
    while (i != end) {
      Connection * connection = *i;

      // Remove the connection and delete if update returns the connection is
      // complete
      result = connection->update(now);
      if (result == ResultCode_t::INCOMPLETE) {
        ++i;
        didSomething = true;
      } else if (result == ResultCode_t::NO_OPERATION) {
        ++i;
      } else {
        if (result == ResultCode_t::TIMEOUT)
          info("Closing connection to " + connection->getEndpoint() +
               " - Timeout");
        else if (!result)
          error("Closing connection to " + connection->getEndpoint() + " - " +
                result.getMessage());
        else
          info("Closing connection to " + connection->getEndpoint() +
               " - Operations completed successfully");

        connection->stop();
        delete connection;
        i = connections.erase(i);
      }
    }

    if (connections.empty()) {
      if (timeoutTime ==
          std::chrono::time_point<std::chrono::system_clock>::min()) {
        timeoutTime = now + TIMEOUT_NO_CONNECTIONS;
      }
    } else {
      timeoutTime = std::chrono::time_point<std::chrono::system_clock>::min();
    }

    // If nothing was processed this loop, sleep until the next to save CPU
    // usage
    if (!didSomething)
      std::this_thread::sleep_for(std::chrono::milliseconds(1));
  }

  // Free socket
  if (socket != nullptr) {
    socket->shutdown(asio::ip::tcp::socket::shutdown_both, errorCode);
    socket->close(errorCode);
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
 * @return Result
 */
Result Server::attachCallback(
    const std::string & uri, const EBInputCallback_t inputCallback) {
  inputCallbacks[uri] = inputCallback;
  return ResultCode_t::SUCCESS;
}

/**
 * @brief Attach a callback to input files originating from the uri
 *
 * @param uri of the source page to subscribe to
 * @param inputCallback function
 * @return Result
 */
Result Server::attachCallback(
    const std::string & uri, const EBInputFileCallback_t inputCallback) {
  inputFileCallbacks[uri] = inputCallback;
  return ResultCode_t::SUCCESS;
}

/**
 * @brief Get the domain name the server is listening to
 *
 * @return const char * domain name: '127.0.0.1:8080'
 */
const char * Server::getDomainName() const {
  return domainName.c_str();
}

/**
 * @brief Check if the server is complete
 *
 * @return true when connections have been idle for greater than the timeout or
 * server is not running
 * @return false otherwise
 */
bool Server::isDone() const {
  return (std::chrono::system_clock::now() > timeoutTime &&
             connections.empty()) ||
         !running;
}

} // namespace Web
} // namespace Ehbanana