#include "WebSocket.h"

#include <spdlog/spdlog.h>

namespace Web {
namespace WebSocket {

/**
 * @brief Construct a new WebSocket::WebSocket object
 *
 */
WebSocket::WebSocket() {}

/**
 * @brief Destroy the WebSocket::WebSocket object
 *
 */
WebSocket::~WebSocket() {}

/**
 * @brief Process a received buffer, could be the entire message or a fragment
 *
 * @param begin character
 * @param length of buffer
 * @return Result error code
 */
Result WebSocket::processReceiveBuffer(const char * begin, size_t length) {
  spdlog::info(std::string(begin, length));
  return ResultCode_t::NOT_SUPPORTED + "WebSocket processReceiveBuffer";
}

/**
 * @brief Check the completion of the protocol
 *
 * @return true if a message is being processed or waiting for a message
 * @return false if all messages have been processed and no more are expected
 */
bool WebSocket::isDone() {
  return false;
}

} // namespace WebSocket
} // namespace Web