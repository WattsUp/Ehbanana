#include "Server.h"
#include "spdlog/spdlog.h"

namespace Web {

Server::Server(const std::string & root, const std::string & addr,
    const std::string & port) :
  ioContext(1),
  acceptor(ioContext), requestHandler(root) {
  // Open the acceptor with the option to reuse the address (i.e. SO_REUSEADDR).
  asio::ip::tcp::resolver resolver(ioContext);
  asio::ip::tcp::endpoint endpoint = *resolver.resolve(addr, port).begin();
  acceptor.open(endpoint.protocol());
  acceptor.set_option(asio::ip::tcp::acceptor::reuse_address(true));
  acceptor.bind(endpoint);
  acceptor.non_blocking(true);
  acceptor.listen();
}

Server::~Server() {
  stop();
}

void Server::start() {
  stop();
  running = true;
  thread  = new std::thread(&Server::run, this);
}

void Server::run() {
  asio::ip::tcp::socket * socket = nullptr;
  asio::error_code        errorCode;
  while (running) {
    // Check for new connections
    if (socket == nullptr)
      socket = new asio::ip::tcp::socket(ioContext);
    acceptor.accept(*socket, errorCode);
    if (!errorCode) {
      spdlog::debug("Accepted a socket");
      connections.push_back(new Connection(socket, &requestHandler));
      socket = nullptr;
    }
    std::this_thread::sleep_for(std::chrono::milliseconds(250));
    spdlog::debug("loop");

    // Process current connections
    std::list<Connection *>::iterator i   = connections.begin();
    std::list<Connection *>::iterator end = connections.end();
    while (i != end) {
      Connection * connection = *i;
      // Remove the connection if update returns the connection is complete
      if (connection->update()) {
        connection->stop();
        i = connections.erase(i);
      } else {
        ++i;
      }
    }
  }
  // Free socket if not already
  if (socket != nullptr) {
    socket->close();
    delete socket;
    socket = nullptr;
  }
}

void Server::stop() {
  running = false;
  if (thread == nullptr)
    return;
  if (thread->joinable())
    thread->join();
  delete thread;
  thread = nullptr;
}

} // namespace Web