#include "Server.h"

#include <spdlog/spdlog.h>
#include <string>

namespace Web {

/**
 * @brief Construct a new Server:: Server object
 *
 * @param httpRoot directory to serve http pages
 * @param configRoot containing the configuration files
 */
Server::Server(const std::string & httpRoot, const std::string & configRoot) :
  ioContext(1), acceptor(ioContext), requestHandler(httpRoot, configRoot) {}

/**
 * @brief Destroy the Server:: Server object
 * Safely stop the thread and any open connections
 */
Server::~Server() {
  stop();
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
Result Server::initialize(const std::string & addr, uint16_t port) {
  asio::ip::tcp::endpoint endpoint;
  try {
    asio::ip::address address = asio::ip::make_address(addr);
    endpoint                  = asio::ip::tcp::endpoint(address, port);
    acceptor.open(endpoint.protocol());
    acceptor.set_option(asio::ip::tcp::acceptor::reuse_address(true));
  } catch (const asio::system_error & e) {
    return ResultCode_t::EXCEPTION_OCCURED + e.what() + "Creating acceptor";
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
    else if (errorCode.value() != asio::error::address_in_use)
      return ResultCode_t::BIND_FAILED +
             ("Server acceptor bind to " + endpoint.address().to_string() +
                 ":" + std::to_string(port));
    // else address already in use, try the next one
    ++port;
  } while (!attemptComplete && port < 65535);

  try {
    acceptor.non_blocking(true);
    acceptor.listen(asio::ip::tcp::socket::max_listen_connections);
  } catch (const asio::system_error & e) {
    return ResultCode_t::EXCEPTION_OCCURED + e.what() +
           "Setting acceptor options";
  }

  requestHandler.setGUIPort(endpoint.port());

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
  bool                    didSomething = false;
  auto                    now          = std::chrono::system_clock::now();
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
      connections.push_back(
          new Connection(socket, endpointString, &requestHandler));
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
      } else if (result == ResultCode_t::NO_OPERATION)
        ++i;
      else {
        if (result == ResultCode_t::TIMEOUT)
          spdlog::warn(result.getMessage());
        else if (!result)
          spdlog::error(result.getMessage());

        spdlog::debug(
            "Closing connection to {}", connection->getEndpointString());
        connection->stop();
        delete connection;
        i = connections.erase(i);
      }
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
 * @brief Get the domain name the server is listening to
 *
 * @return std::string& domain name: '127.0.0.1:8080'
 */
const std::string & Server::getDomainName() const {
  return domainName;
}

} // namespace Web