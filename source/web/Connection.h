#ifndef _WEB_CONNECTION_H_
#define _WEB_CONNECTION_H_

#include "Reply.h"
#include "Request.h"
#include "RequestHandler.h"

#include <FruitBowl.h>
#include <asio.hpp>

#include <array>
#include <chrono>
#include <stdint.h>
#include <string>

namespace Web {

class Connection {
public:
  Connection(const Connection &) = delete;
  Connection & operator=(const Connection &) = delete;

  Connection(asio::ip::tcp::socket * socket, std::string endpoint,
      RequestHandler * requestHandler);
  ~Connection();

  Result update(const std::chrono::time_point<std::chrono::system_clock> & now);
  void   stop();

  std::string getEndpointString() const;

private:
  Result updateHTTP(
      const std::chrono::time_point<std::chrono::system_clock> & now);
  Result updateWebSocket(
      const std::chrono::time_point<std::chrono::system_clock> & now);

  Result read();
  Result write();

  enum class State_t : uint8_t {
    IDLE,
    READING,
    READING_DONE,
    WRITING,
    WRITING_DONE,
    COMPLETE
  };

  enum class Protocol_t : uint16_t { HTTP, WEBSOCKET };

  asio::ip::tcp::socket * socket;
  std::string             endpoint;
  RequestHandler *        requestHandler;

  Reply                  reply;
  Request                request;
  std::array<char, 8192> buffer;
  State_t                state = State_t::IDLE;

  Protocol_t protocol = Protocol_t::HTTP;

  std::chrono::time_point<std::chrono::system_clock> timeoutTime;

  const std::chrono::seconds TIMEOUT {60};
};

} // namespace Web

#endif /* _WEB_CONNECTION_H_ */