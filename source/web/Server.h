#ifndef _WEB_SERVER_H_
#define _WEB_SERVER_H_

#include <FruitBowl.h>
#include <asio.hpp>

#include <atomic>
#include <chrono>
#include <list>
#include <stdint.h>
#include <string>
#include <thread>

#include "Connection.h"
#include "RequestHandler.h"

namespace Web {

class Server {
public:
  Server(const Server &) = delete;
  Server & operator=(const Server &) = delete;

  Server(const std::string & httpRoot, const std::string & configRoot);
  ~Server();

  Result initialize(const std::string & addr, uint16_t port = PORT_AUTO);
  void   start();
  void   stop();

  void setGUIPort(uint16_t port);

  const std::string & getDomainName() const;

  static const uint16_t PORT_AUTO    = 0;
  static const uint16_t PORT_DEFAULT = 8080;

private:
  void run();

  std::thread *     thread  = nullptr;
  std::atomic<bool> running = false;

  asio::io_context        ioContext;
  asio::ip::tcp::acceptor acceptor;

  std::string domainName;

  RequestHandler requestHandler;

  std::list<Connection *> connections;
};

} // namespace Web

#endif /* _WEB_SERVER_H_ */