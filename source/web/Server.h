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
#include <unordered_map>

namespace Ehbanana {
namespace Web {

class Server {
public:
  Server(const Server &) = delete;
  Server & operator=(const Server &) = delete;

  Server();
  ~Server();

  Result initialize(const EBGUISettings_t settings);
  Result start();
  void   stop();

  bool isDone() const;

  Result attachCallback(
      const std::string & uri, const EBInputCallback_t inputCallback);
  Result attachCallback(
      const std::string & uri, const EBInputFileCallback_t inputFileCallback);

  const char * getDomainName() const;

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

  std::unordered_map<std::string, EBInputCallback_t>     inputCallbacks;
  std::unordered_map<std::string, EBInputFileCallback_t> inputFileCallbacks;

  std::chrono::time_point<std::chrono::system_clock> timeoutTime;
  std::chrono::seconds                               TIMEOUT_NO_CONNECTIONS;
};

} // namespace Web
} // namespace Ehbanana

#endif /* _WEB_SERVER_H_ */