#include "Server.h"
#include "spdlog/spdlog.h"

namespace Web {

/**
 * @brief Construct a new Server:: Server object
 *
 * @param root directory to serve http pages
 */
Server::Server(const std::string & root) :
  ioContext(1), acceptor(ioContext), requestHandler(root) {}

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
 * Passing in 0 for port will attempt to use the default port then increment
 * until an available port is open
 *
 * @param addr ip address to bind to
 * @param port to bind to
 * @return Results::Result_t
 */
Results::Result_t Server::initialize(const std::string & addr, uint16_t port) {
  asio::error_code  errorCode;
  asio::ip::address address = asio::ip::make_address(addr, errorCode);
  if (errorCode)
    return Results::INVALID_PARAMETER + ("Making address from " + addr) +
           errorCode.message();
  asio::ip::tcp::endpoint endpoint(address, port);

  acceptor.open(endpoint.protocol(), errorCode);
  if (errorCode)
    return Results::OPEN_FAILED + "Opening acceptor" + errorCode.message();

  acceptor.set_option(asio::ip::tcp::acceptor::reuse_address(true), errorCode);
  if (errorCode)
    return Results::INVALID_PARAMETER + "Setting option resuse address" +
           errorCode.message();

  // Attempt to bind until an available port is found
  bool attemptComplete = true;
  if (port == 0) {
    port            = DEFAULT_PORT;
    attemptComplete = false;
  }
  do {
    endpoint.port(port);
    spdlog::debug("Binding acceptor to {}", endpoint);
    acceptor.bind(endpoint, errorCode);
    if (!errorCode)
      attemptComplete = true;
    else if (errorCode.value() != asio::error::address_in_use)
      return Results::OPEN_FAILED + std::to_string(errorCode.value()) +
             errorCode.message();
    // else address already in use, try the next one
    ++port;
  } while (!attemptComplete && port < 65535);
  spdlog::info("Server bound to {}", endpoint);

  acceptor.non_blocking(true, errorCode);
  if (errorCode)
    return Results::INVALID_PARAMETER + "Setting acceptor non-blocking" +
           errorCode.message();

  acceptor.listen(asio::ip::tcp::socket::max_listen_connections, errorCode);
  if (errorCode)
    return Results::OPEN_FAILED + "Setting acceptor in listening mode" +
           errorCode.message();
  return Results::SUCCESS;
}

/**
 * @brief Start the run thread
 *
 */
void Server::start() {
  stop();
  spdlog::info("Server starting");
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
  Results::Result_t       result       = Results::SUCCESS;
  bool                    didSomething = false;
  while (running) {
    didSomething = false;
    // Check for new connections
    if (socket == nullptr)
      socket = new asio::ip::tcp::socket(ioContext);
    acceptor.accept(*socket, endpoint, errorCode);
    if (!errorCode) {
      spdlog::debug("Accepted a connection from {}", endpoint);
      connections.push_back(new Connection(socket, endpoint, &requestHandler));
      socket       = nullptr;
      didSomething = true;
    } else if (errorCode != asio::error::would_block) {
      spdlog::error("Acceptor encountered an error: {}", errorCode);
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
      result = connection->update();
      if (result == Results::INCOMPLETE_OPERATION) {
        ++i;
        didSomething = true;
      } else {
        if (!result)
          spdlog::error(result);

        spdlog::debug("Closing connection to {}", connection->getEndpoint());
        connection->stop();
        i = connections.erase(i);
      }
    }

    // If nothing was processed this loop, sleep until the next to save CPU
    // usage
    if (!didSomething)
      std::this_thread::sleep_for(std::chrono::microseconds(100));
  }
  // Free socket
  if (socket != nullptr) {
    socket->close();
    delete socket;
    socket = nullptr;
  }
}

/**
 * @brief Stop the run thread
 *
 */
void Server::stop() {
  if (running)
    spdlog::info("Server stopping");
  running = false;
  if (thread == nullptr)
    return;
  if (thread->joinable())
    thread->join();
  delete thread;
  thread = nullptr;
}

} // namespace Web