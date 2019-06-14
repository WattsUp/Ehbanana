#include "Connection.h"
#include "spdlog/spdlog.h"

namespace Web {

Connection::Connection(
    asio::ip::tcp::socket * socket, RequestHandler * requestHandler) {
  this->socket         = socket;
  this->requestHandler = requestHandler;
}

Connection::~Connection() {
  stop();
}

void Connection::start() {
  read();
}

bool Connection::update() {
  return true;
}

void Connection::stop() {
  if (socket != nullptr)
    socket->close();
  delete socket;
  socket = nullptr;
}

void Connection::read() {}

void Connection::write() {}

} // namespace Web