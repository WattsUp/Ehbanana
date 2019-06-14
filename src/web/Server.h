#ifndef _WEB_SERVER_H_
#define _WEB_SERVER_H_

#include <asio.hpp>

#include <list>
#include <stdint.h>
#include <string>

#include "Connection.h"
#include "RequestHandler.h"
#include "Result.h"

namespace Web {

static const char * DEFAULT_ROOT      = "http";
static const char * DEFAULT_ADDR      = "127.0.0.1";
static const char * DEFAULT_PORT_HTTP = "8080";

class Server {
public:
  Server(const Server &) = delete;
  Server & operator=(const Server &) = delete;

  Server(const std::string & root = DEFAULT_ROOT,
      const std::string &    addr = DEFAULT_ADDR,
      const std::string &    port = DEFAULT_PORT_HTTP);
  ~Server();

  Results::Result_t run();
  Results::Result_t stop();

  void stopConnection(Connection * connection);
  void stopConnections();

private:
  void startConnection(Connection * connection);

  asio::io_context        ioContext;
  asio::ip::tcp::acceptor acceptor;

  RequestHandler requestHandler;

  std::list<Connection *> connections;
};

} // namespace Web

#endif /* _WEB_SERVER_H_ */