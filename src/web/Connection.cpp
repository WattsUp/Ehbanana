#include "Connection.h"
#include "Server.h"
#include "spdlog/spdlog.h"

namespace Web {

Connection::Connection(asio::ip::tcp::socket socket, Server * server,
    RequestHandler * requestHandler) :
  socket(std::move(socket)) {
  this->server         = server;
  this->requestHandler = requestHandler;
}

void Connection::start() {
  read();
}

void Connection::stop() {
  socket.close();
}

void Connection::read() {
  std::shared_ptr<Connection> self(shared_from_this());
  socket.async_read_some(asio::buffer(buffer),
      [this, self](std::error_code ec, std::size_t bytes_transferred) {
        if (!ec) {
          Results::Result_t result =
              request.parse(buffer.data(), buffer.data() + bytes_transferred);
          if (result == Results::SUCCESS) {
            // Handle request
            write();
          } else if (result == Results::INCOMPLETE_OPERATION) {
            read();
          } else {
            // Reply with bad request

            reply.setStatus(HTTPStatus::OK);
            reply.appendContent("<html>Hello World</html>");
            reply.addHeader(
                "Content-Length", std::to_string(reply.content.size()));
            reply.addHeader("Content-Type", "text/html");
#ifdef DEBUG
            spdlog::error(result + "Encountered a bad HTTP request");
#endif
            write();
          }
        } else if (ec != asio::error::operation_aborted) {
          server->stopConnection(this);
        }
      });
}

void Connection::write() {
  std::shared_ptr<Connection> self(shared_from_this());

  std::vector<asio::const_buffer> buffers;
  buffers.push_back(asio::buffer(
      "HTTP/1.1 200 OK\r\nContent-Type: text/html\r\n\r\nHello World\r\n"));

  asio::async_write(
      socket, buffers, [this, self](std::error_code ec, std::size_t) {
        if (!ec) {
          // Initiate graceful connection closure.
          asio::error_code ignoredErrorCode;
          socket.shutdown(
              asio::ip::tcp::socket::shutdown_both, ignoredErrorCode);
        }

        if (ec != asio::error::operation_aborted) {
          server->stopConnection(this);
        }
      });
}

} // namespace Web