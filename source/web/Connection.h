#ifndef _WEB_CONNECTION_H_
#define _WEB_CONNECTION_H_

#include "AppProtocol.h"
#include "Ehbanana.h"
#include "HTTP/HTTP.h"
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
      const timepoint_t<sysclk_t> & now);
  ~Connection();

  void update(const timepoint_t<sysclk_t> & now);
  void stop();

  const std::string & toString() const;

  enum class State_t : uint8_t { IDLE, BUSY, DONE };

  State_t getState() const;

private:
  std::unique_ptr<Net::socket_t> socket;
  std::string                    endpoint;

  std::array<uint8_t, 8192> bufferReceive;

  State_t state = State_t::IDLE;

  std::unique_ptr<AppProtocol> protocol = std::make_unique<HTTP::HTTP>();

  timepoint_t<sysclk_t> timeoutTime;

  const seconds_t TIMEOUT {1};
};

} // namespace Web
} // namespace Ehbanana

#endif /* _WEB_CONNECTION_H_ */