#ifndef _WEB_CONNECTION_H_
#define _WEB_CONNECTION_H_

#include "AppProtocol.h"
#include "HTTP/HTTP.h"
#include "WebSocket/WebSocket.h"

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
      const std::chrono::time_point<std::chrono::system_clock> & now);
  ~Connection();

  Result update(const std::chrono::time_point<std::chrono::system_clock> & now);
  void   stop();

  const std::string & getEndpoint() const;

private:
  asio::ip::tcp::socket * socket;
  std::string             endpoint;

  std::array<char, 8192> bufferReceive;

  AppProtocol * protocol = new HTTP::HTTP();

  std::chrono::time_point<std::chrono::system_clock> timeoutTime;

  const std::chrono::seconds TIMEOUT {10};
};

} // namespace Web

#endif /* _WEB_CONNECTION_H_ */