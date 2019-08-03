#ifndef _WEB_CONNECTION_HTTP_H_
#define _WEB_CONNECTION_HTTP_H_

#include "Connection.h"
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

class ConnectionHTTP : public Connection {
public:
  ConnectionHTTP(const ConnectionHTTP &) = delete;
  ConnectionHTTP & operator=(const ConnectionHTTP &) = delete;

  ConnectionHTTP(asio::ip::tcp::socket * socket, std::string endpoint,
      RequestHandler * requestHandler);

  Result update(const std::chrono::time_point<std::chrono::system_clock> & now);

private:
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

  RequestHandler * requestHandler;

  Reply   reply;
  Request request;
  State_t state = State_t::IDLE;
};

} // namespace Web

#endif /* _WEB_CONNECTION_H_ */