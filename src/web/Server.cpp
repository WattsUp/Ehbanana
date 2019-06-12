#include "Server.h"
#include "spdlog/spdlog.h"

namespace Web {

Server::Server(const std::string & root, const std::string & addr,
    const std::string & port) :
  ioContext(1),
  signals(ioContext), acceptor(ioContext), requestHandler(root) {
  // Register to handle the signals that indicate when the server should exit.
  // It is safe to register for the same signal multiple times in a program,
  // provided all registration for the specified signal is made through Asio.
  signals.add(SIGINT);
  signals.add(SIGTERM);

  stop();

  // Open the acceptor with the option to reuse the address (i.e. SO_REUSEADDR).
  asio::ip::tcp::resolver resolver(ioContext);
  asio::ip::tcp::endpoint endpoint = *resolver.resolve(addr, port).begin();
  acceptor.open(endpoint.protocol());
  acceptor.set_option(asio::ip::tcp::acceptor::reuse_address(true));
  acceptor.bind(endpoint);
  acceptor.listen();

  accept();
}

void Server::accept() {
  acceptor.async_accept(
      [this](std::error_code ec, asio::ip::tcp::socket socket) {
        // Check whether the server was stopped by a signal before this
        // completion handler had a chance to run.
        if (!acceptor.is_open()) {
          return;
        }

        if (!ec) {
          startConnection(std::make_shared<Connection>(
              std::move(socket), this, &requestHandler));
        }

        accept();
      });
}

void Server::stopConnection(std::shared_ptr<Connection> connection) {
  connections.erase(connection);
  connection->stop();
}

void Server::stopConnections() {
  for (std::shared_ptr<Connection> connection : connections)
    connection->stop();
  connections.clear();
}

void Server::startConnection(std::shared_ptr<Connection> connection) {
  connections.insert(connection);
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