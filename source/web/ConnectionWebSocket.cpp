#include "ConnectionWebSocket.h"

#include <spdlog/spdlog.h>

namespace Web {

/**
 * @brief Construct a new ConnectionWS:: ConnectionWS object
 *
 * @param socket to read from and write to
 * @param endpoint socket is connected to
 */
ConnectionWS::ConnectionWS(
    asio::ip::tcp::socket * socket, std::string endpoint) :
  Connection(socket, endpoint) {
  protocol = Protocol_t::WEBSOCKET;
}

/**
 * @brief Update the current operation, read or write
 *
 * Returns ResultCode_t::INCOMPLETE if more operations are required
 * Returns ResultCode_t::NO_OPERATION if nothing happenend this update
 * Returns ResultCode_t::TIMEOUT if the connection was idle for too long
 *
 * @param now current timestamp
 * @return Result error code
 */
Result ConnectionWS::update(
    const std::chrono::time_point<std::chrono::system_clock> & now) {
  spdlog::info("Connection using web socket");
  return ResultCode_t::SUCCESS;
}

} // namespace Web