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
 * @return EBResultMsg_t error code
 */
EBResultMsg_t Server::initialize(const std::string & addr, uint16_t port) {
  asio::error_code  errorCode;
  asio::ip::address address = asio::ip::make_address(addr, errorCode);
  if (errorCode)
    return EBResult::ASIO_MAKE_ADDRESS + ("From: " + addr);

  asio::ip::tcp::endpoint endpoint(address, port);
  acceptor.open(endpoint.protocol(), errorCode);
  if (errorCode)
    return EBResult::ASIO_OPEN_ACCEPTOR + "Server acceptor open";

  acceptor.set_option(asio::ip::tcp::acceptor::reuse_address(true), errorCode);
  if (errorCode)
    return EBResult::ASIO_SET_OPTION + "Server acceptor reuse address";

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
      return EBResult::ASIO_BIND +
             ("Server acceptor bind to " + endpoint.address().to_string() +
                 ":" + std::to_string(port));
    // else address already in use, try the next one
    ++port;
  } while (!attemptComplete && port < 65535);

  acceptor.non_blocking(true, errorCode);
  if (errorCode)
    return EBResult::ASIO_SET_OPTION + "Server acceptor non-blocking";

  acceptor.listen(asio::ip::tcp::socket::max_listen_connections, errorCode);
  if (errorCode)
    return EBResult::ASIO_LISTEN + "Server acceptor listen";

  domainName =
      endpoint.address().to_string() + ":" + std::to_string(endpoint.port());

  return EBResult::SUCCESS;
}

/**
 * @brief Start the run thread
 *
 * @return EBResultMsg_t error code
 */
EBResultMsg_t Server::start() {
  stop();
  running = true;
  thread  = new std::thread(&Server::run, this);
  return EBResult::SUCCESS;
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
  EBResultMsg_t           result       = EBResult::SUCCESS;
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
      connections.push_back(new Connection(socket, endpoint, &requestHandler));
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
      // Remove the connection if update returns the connection is complete
      result = connection->update(now);
      if (result == EBResult::INCOMPLETE_OPERATION) {
        ++i;
        didSomething = true;
      } else if (result == EBResult::NO_OPERATION)
        ++i;
      else {
        if (result == EBResult::TIMEOUT)
          spdlog::warn(result);
        else if (!result)
          spdlog::error(result);

        // spdlog::debug("Closing connection to {}", connection->getEndpoint());
        connection->stop();
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
 * @param EBResultMsg_t error code
 */
EBResultMsg_t Server::stop() {
  running = false;
  if (thread == nullptr)
    return EBResult::SUCCESS;
  if (thread->joinable())
    thread->join();
  delete thread;
  thread = nullptr;
  return EBResult::SUCCESS;
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