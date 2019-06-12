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
          // request_parser::result_type result;
          // std::tie(result, std::ignore) = request_parser_.parse(
          //     request_, buffer_.data(), buffer_.data() + bytes_transferred);

          // if (result == request_parser::good)
          // {
          //   request_handler_.handle_request(request_, reply_);
          //   do_write();
          // }
          // else if (result == request_parser::bad)
          // {
          //   reply_ = reply::stock_reply(reply::bad_request);
            // do_write();
          // }
          // else
          // {
          //   read();
          // }
          spdlog::debug("Connection recieved: |{}|", buffer.data());
          write();
        } else if (ec != asio::error::operation_aborted) {
          server->stopConnection(shared_from_this());
        }
      });
}

void Connection::write() {
  std::shared_ptr<Connection>     self(shared_from_this());
  std::vector<asio::const_buffer> buffers;
  buffers.push_back(asio::buffer("HTTP/1.1 200 OK\r\nContent-Type: text/plain\r\n\r\nHello World\r\n"));

  asio::async_write(
      socket, buffers, [this, self](std::error_code ec, std::size_t) {
        if (!ec) {
          // Initiate graceful connection closure.
          asio::error_code ignored_ec;
          socket.shutdown(asio::ip::tcp::socket::shutdown_both, ignored_ec);
        }

        if (ec != asio::error::operation_aborted) {
          server->stopConnection(shared_from_this());
        }
      });
}

} // namespace Web