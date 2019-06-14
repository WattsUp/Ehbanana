#ifndef _WEB_CONNECTION_H_
#define _WEB_CONNECTION_H_

#include "Reply.h"
#include "Request.h"
#include "RequestHandler.h"
#include "Result.h"

#include <asio.hpp>

#include <array>
#include <stdint.h>
#include <string>

namespace Web {

class Connection {
public:
  Connection(const Connection &) = delete;
  Connection & operator=(const Connection &) = delete;

  Connection(asio::ip::tcp::socket * socket, RequestHandler * requestHandler);
  ~Connection();

  void start();
  bool update();
  void stop();

private:
  void read();
  void write();

  asio::ip::tcp::socket * socket;
  RequestHandler *        requestHandler;

  Reply                  reply;
  Request                request;
  std::array<char, 8192> buffer;
};

} // namespace Web

#endif /* _WEB_CONNECTION_H_ */