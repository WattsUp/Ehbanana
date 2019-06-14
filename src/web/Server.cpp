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
  acceptor.listen();
}

Server::~Server() {
  stopConnections();
}


void Server::stopConnection(Connection * connection) {
  connection->stop();
  delete connection;
  connections.remove(connection);
}

void Server::stopConnections() {
  while (!connections.empty()) {
    connections.front()->stop();
    delete connections.front();
    connections.pop_front();
  }
}

void Server::startConnection(Connection * connection) {
  connections.push_back(connection);
  connection->start();
}

Results::Result_t Server::run() {
  ioContext.run();
  return Results::SUCCESS;
}

Results::Result_t Server::stop() {
  signals.async_wait([this](std::error_code /*ec*/, int /*signo*/) {
    // The server is stopped by cancelling all outstanding asynchronous
    // operations. Once all operations have finished the io_context::run()
    // call will exit.
    acceptor.close();
    stopConnections();
  });
  return Results::SUCCESS;
}

} // namespace Web