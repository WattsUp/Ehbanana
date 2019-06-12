#ifndef _WEB_SERVER_H_
#define _WEB_SERVER_H_

#include "Connection.h"
#include "RequestHandler.h"
#include "Result.h"

#include <asio.hpp>

#include <memory>
#include <set>
#include <stdint.h>
#include <string>

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

  Results::Result_t run();
  Results::Result_t stop();

  void stopConnection(std::shared_ptr<Connection> connection);
  void stopConnections();

private:
  void accept();
  void startConnection(std::shared_ptr<Connection> connection);

  asio::io_context        ioContext;
  asio::signal_set        signals;
  asio::ip::tcp::acceptor acceptor;

  RequestHandler requestHandler;

  std::set<std::shared_ptr<Connection> > connections;
};

} // namespace Web

#endif /* _WEB_SERVER_H_ */