#ifndef _WEB_CONNECTION_H_
#define _WEB_CONNECTION_H_

#include "Reply.h"
#include "Request.h"
#include "RequestHandler.h"
#include "Result.h"

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

  Results::Result_t update(
      const std::chrono::time_point<std::chrono::system_clock> & now);
  void stop();

  asio::ip::tcp::endpoint getEndpoint();

private:
  Results::Result_t read();
  Results::Result_t write();

  typedef enum State {
    IDLE,
    READING,
    READING_DONE,
    WRITING,
    WRITING_DONE,
    COMPLETE
  } State_t;

  asio::ip::tcp::socket * socket;
  asio::ip::tcp::endpoint endpoint;
  RequestHandler *        requestHandler;

  Reply                  reply;
  Request                request;
  std::array<char, 8192> buffer;
  State_t                state = IDLE;

  std::chrono::time_point<std::chrono::system_clock> timeoutTime;

  const std::chrono::seconds TIMEOUT {60};
};

} // namespace Web

#endif /* _WEB_CONNECTION_H_ */