#ifndef _WEB_CONNECTION_H_
#define _WEB_CONNECTION_H_

#include "AppProtocol.h"
#include "Ehbanana.h"
#include "HTTP/HTTP.h"
#include "MessageOut.h"
#include "Utils.h"
#include "WebSocket/WebSocket.h"

#include <asio.hpp>

#include <array>
#include <chrono>
#include <memory>
#include <stdint.h>
#include <string>

namespace Ehbanana {
namespace Web {

class Connection {
public:
  Connection(const Connection &) = delete;
  Connection & operator=(const Connection &) = delete;

  Connection(std::unique_ptr<Net::socket_t> socket, std::string endpoint,
      const timepoint_t<sysclk_t> & now, Server * server);
  ~Connection();

  void update(const timepoint_t<sysclk_t> & now);
  void stop();

  void enqueueOutput(std::shared_ptr<Ehbanana::MessageOut> message);

  const std::string & toString() const;

  enum class State_t : uint8_t { IDLE, BUSY, DONE };

  State_t getState() const;

private:
  std::unique_ptr<Net::socket_t> socket;
  std::string                    endpoint;

  Server * server;

  std::array<uint8_t, 0x4000> bufferReceive;

  State_t state = State_t::IDLE;

  std::unique_ptr<AppProtocol> protocol;

  timepoint_t<sysclk_t> timeoutTime;

  const seconds_t TIMEOUT {1};
};

} // namespace Web
} // namespace Ehbanana

#endif /* _WEB_CONNECTION_H_ */