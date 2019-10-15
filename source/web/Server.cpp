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
 * @param gui that owns this server
 * @param timeoutIdle in seconds to wait before exiting when no connections are
 * in progress (allows time for browser to load new pages)
 * @param timeoutFirst in seconds to wait before exiting when the first
 * connection is in progress (allows time for browser to boot)
 */
Server::Server(EBGUI_t gui, uint8_t timeoutIdle, uint8_t timeoutFirst) :
  ioContext(1), acceptor(ioContext), gui(gui),
  TIMEOUT_NO_CONNECTIONS(timeoutIdle), TIMEOUT_FIRST_CONNECTIONS(timeoutFirst) {
}

/**
 * @brief Destroy the Server:: Server object
 * Safely stop the thread and any open connections
 */
Server::~Server() {
  stop();
}

/**
 * @brief Configure settings of the server, folder directories
 *
 * @param httpRoot directory to serve http pages
 * @param configRoot containing the configuration files
 * @return Result error code
 */
Result Server::configure(
    const std::string & httpRoot, const std::string & configRoot) {
  Result result;

  HTTP::HTTP::setRoot(httpRoot);
  result =
      HTTP::CacheControl::Instance()->populateList(configRoot + "/cache.xml");
  if (!result)
    return result + "Configuring server's cache control";
  result =
      HTTP::MIMETypes::Instance()->populateList(configRoot + "/mime.types");
  if (!result)
    return result + "Configuring server's mime types";
  return ResultCode_t::SUCCESS;
}

/**
 * @brief Initialize the server's acceptor
 * Opens the acceptor at the desired address and places it into listening mode
 * Passing in PORT_AUTO for port will attempt to use the default port then
 * increment until an available port is open
 *
 * @param addr ip address to bind to
 * @param port to bind to
 * @return Result error code
 */
Result Server::initializeSocket(const std::string & addr, uint16_t port) {
  asio::ip::tcp::endpoint endpoint;
  try {
    asio::ip::address address = asio::ip::make_address(addr);
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
  asio::ip::tcp::socket * socket = nullptr;
  asio::ip::tcp::endpoint endpoint;
  asio::error_code        errorCode;
  Result                  result;
  bool                    didSomething            = false;
  bool                    outputMessageDispatched = false;

  auto now         = std::chrono::system_clock::now();
  auto timeoutTime = std::chrono::time_point<std::chrono::system_clock>::min();

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
      connections.push_back(new Connection(socket, endpointString, now, gui));
      socket              = nullptr;
      didSomething        = true;
      firstConnectionMade = true;
    } else if (errorCode != asio::error::would_block) {
      running      = false;
      didSomething = true;
    }
    // else no waiting connections

    // Process current connections
    std::list<Connection *>::iterator i   = connections.begin();
    std::list<Connection *>::iterator end = connections.end();
    outputMessageDispatched               = false;
    while (i != end) {
      Connection * connection = *i;

      // Add the next output message if available
      if (!outputMessages.empty()) {
        result = connection->addMessage(outputMessages.front());
        if (result)
          outputMessageDispatched = true;
      }

      // Remove the connection and delete if update returns the connection is
      // complete
      result = connection->update(now);
      if (result == ResultCode_t::INCOMPLETE) {
        ++i;
        didSomething = true;
      } else if (result == ResultCode_t::NO_OPERATION)
        ++i;
      else {
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
        timeoutTime = now + (firstConnectionMade ? TIMEOUT_NO_CONNECTIONS
                                                 : TIMEOUT_FIRST_CONNECTIONS);
      }
      if (now > timeoutTime) {
        // Server had no connections for the timeout time
        EBEnqueueMessage({gui, EBMSGType_t::SHUTDOWN});
        EBEnqueueMessage({gui, EBMSGType_t::QUIT});
      }
    } else {
      timeoutTime = std::chrono::time_point<std::chrono::system_clock>::min();
    }

    if (outputMessageDispatched)
      outputMessages.pop_front();

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
    socket = nullptr;
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
 * @brief Enqueue a message to output to connected websockets
 *
 * @param msg to enqueue
 */
void Server::enqueueOutput(const std::string & msg) {
  outputMessages.push_back(msg);
}

/**
 * @brief Get the domain name the server is listening to
 *
 * @return std::string& domain name: '127.0.0.1:8080'
 */
const std::string & Server::getDomainName() const {
  return domainName;
}

} // namespace Web
} // namespace Ehbanana