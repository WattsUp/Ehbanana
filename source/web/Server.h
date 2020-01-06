#ifndef _WEB_SERVER_H_
#define _WEB_SERVER_H_

#include "Connection.h"
#include "Ehbanana.h"

#include "Utils.h"

#include <asio.hpp>
#include <ctpl_stl.h>

#include <atomic>
#include <chrono>
#include <list>
#include <memory>
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

  Server(const EBGUISettings_t settings);
  ~Server();

  void start();
  void stop();

  bool isDone() const;

  void attachCallback(
      const std::string & uri, const EBInputCallback_t callback);
  void attachCallback(
      const std::string & uri, const EBInputFileCallback_t callback);
  void setOutputCallback(const EBOutputFileCallback_t callback);

  void enqueueCallback(const std::string & uri, const std::string & id,
      const std::string & value);

  const std::string & getDomainName() const;

  static const uint16_t PORT_AUTO    = 0;
  static const uint16_t PORT_DEFAULT = 8080;

private:
  void run();

  std::unique_ptr<std::thread> thread  = nullptr;
  std::atomic<bool>            running = false;
  ctpl::thread_pool            pool;

  asio::io_context ioContext;
  Net::acceptor_t  acceptor;

  std::string domainName;

  std::list<std::unique_ptr<Connection> > connections;

  std::unordered_map<std::string, EBInputCallback_t>     inputCallbacks;
  std::unordered_map<std::string, EBInputFileCallback_t> inputFileCallbacks;
  EBOutputFileCallback_t outputFileCallback = nullptr;

  timepoint_t<sysclk_t> timeoutTime;
  seconds_t             TIMEOUT_NO_CONNECTIONS;
};

} // namespace Web
} // namespace Ehbanana

#endif /* _WEB_SERVER_H_ */