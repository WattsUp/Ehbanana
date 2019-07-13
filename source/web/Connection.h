#ifndef _WEB_CONNECTION_H_
#define _WEB_CONNECTION_H_

#include "Reply.h"
#include "Request.h"
#include "RequestHandler.h"
#include "ResultMsg.h"

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

  Connection(asio::ip::tcp::socket * socket, asio::ip::tcp::endpoint endpoint,
      RequestHandler * requestHandler);
  ~Connection();

  EBResultMsg_t update(
      const std::chrono::time_point<std::chrono::system_clock> & now);
  void stop();

  std::string getEndpointString() const;

private:
  EBResultMsg_t read();
  EBResultMsg_t write();

  enum class State_t : uint8_t {
    IDLE,
    READING,
    READING_DONE,
    WRITING,
    WRITING_DONE,
    COMPLETE
  };

  asio::ip::tcp::socket * socket;
  asio::ip::tcp::endpoint endpoint;
  RequestHandler *        requestHandler;

  Reply                  reply;
  Request                request;
  std::array<char, 8192> buffer;
  State_t                state = State_t::IDLE;

  std::chrono::time_point<std::chrono::system_clock> timeoutTime;

  const std::chrono::seconds TIMEOUT {60};
};

} // namespace Web

#endif /* _WEB_CONNECTION_H_ */