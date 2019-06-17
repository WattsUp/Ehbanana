#ifndef _WEB_SERVER_H_
#define _WEB_SERVER_H_

#include <asio.hpp>

#include <atomic>
#include <chrono>
#include <list>
#include <stdint.h>
#include <string>
#include <thread>

#include "Connection.h"
#include "RequestHandler.h"
#include "Result.h"

namespace Web {

static const char *   DEFAULT_ROOT = "http";
static const char *   DEFAULT_ADDR = "127.0.0.1";
static const uint16_t DEFAULT_PORT = 8080;

class Server {
public:
  Server(const Server &) = delete;
  Server & operator=(const Server &) = delete;

  Server(const std::string & root = DEFAULT_ROOT);
  ~Server();

  Results::Result_t initialize(
      const std::string & addr = DEFAULT_ADDR, uint16_t port = 0);
  void start();
  void stop();

private:
  void run();

  std::thread *     thread = nullptr;
  std::atomic<bool> running = false;

  asio::io_context        ioContext;
  asio::ip::tcp::acceptor acceptor;

  RequestHandler requestHandler;

  std::list<Connection *> connections;
};

} // namespace Web

#endif /* _WEB_SERVER_H_ */