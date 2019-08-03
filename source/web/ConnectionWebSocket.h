#ifndef _WEB_CONNECTION_WEB_SOCKET_H_
#define _WEB_CONNECTION_WEB_SOCKET_H_

#include "Connection.h"

#include <FruitBowl.h>
#include <asio.hpp>

#include <array>
#include <chrono>
#include <stdint.h>
#include <string>

namespace Web {

class ConnectionWS : public Connection {
public:
  ConnectionWS(const ConnectionWS &) = delete;
  ConnectionWS & operator=(const ConnectionWS &) = delete;

  ConnectionWS(asio::ip::tcp::socket * socket, std::string endpoint);

  Result update(const std::chrono::time_point<std::chrono::system_clock> & now);

private:
};

} // namespace Web

#endif /* _WEB_CONNECTION_H_ */