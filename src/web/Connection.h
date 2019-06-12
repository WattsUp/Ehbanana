#ifndef _WEB_CONNECTION_H_
#define _WEB_CONNECTION_H_

#include "RequestHandler.h"
#include "Result.h"

#include <asio.hpp>

#include <array>
#include <memory>
#include <stdint.h>
#include <string>

namespace Web {

class Server;

class Connection
  : public std::enable_shared_from_this<Connection> {
public:
  Connection(const Connection &) = delete;
  Connection & operator=(const Connection &) = delete;

  Connection(asio::ip::tcp::socket socket, Server * server,
      RequestHandler * requestHandler);

  void start();
  void stop();

private:
  void read();
  void write();

  asio::ip::tcp::socket socket;

  Server *         server;
  RequestHandler * requestHandler;

  std::array<char, 8192> buffer;
};

} // namespace Web

#endif /* _WEB_CONNECTION_H_ */