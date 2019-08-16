#ifndef _WEB_SERVER_H_
#define _WEB_SERVER_H_

#include "Connection.h"
#include "Ehbanana.h"

#include <FruitBowl.h>
#include <asio.hpp>

#include <atomic>
#include <chrono>
#include <list>
#include <stdint.h>
#include <string>
#include <thread>

namespace Web {

class Server {
public:
  Server(const Server &) = delete;
  Server & operator=(const Server &) = delete;

  Server(EBGUI_t gui);
  ~Server();

  Result configure(
      const std::string & httpRoot, const std::string & configRoot);
  Result initializeSocket(const std::string & addr, uint16_t port = PORT_AUTO);
  void   start();
  void   stop();

  void enqueueOutput(const EBMessage_t & msg);

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

  std::list<Connection *> connections;
  std::list<EBMessage_t>  outputMessages;

  EBGUI_t gui;

  const std::chrono::seconds TIMEOUT_NO_CONNECTIONS {3};
};

} // namespace Web

#endif /* _WEB_SERVER_H_ */