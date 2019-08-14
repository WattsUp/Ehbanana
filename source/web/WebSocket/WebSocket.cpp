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
Result WebSocket::processReceiveBuffer(const uint8_t * begin, size_t length) {
  Result result = frame.decode(begin, length);
  if (!result)
    return result;

  switch (frame.getOpcode()) {
    case Opcode_t::TEXT:
      spdlog::debug("WebSocket received text: \"{}\"", frame.getData());
      break;
    case Opcode_t::BINARY:
      fseek(frame.getDataFile(), 0L, SEEK_END);
      spdlog::debug("WebSocket received binary of length: {}", ftell(frame.getDataFile()));
      break;
    case Opcode_t::PING:
      spdlog::debug("WebSocket received ping");
      // Send pong
      frame = Frame();
      frame.setOpcode(Opcode_t::PONG);
      addTransmitBuffer(frame.toBuffer());
      return ResultCode_t::INCOMPLETE;
    case Opcode_t::PONG:
      spdlog::debug("WebSocket received pong");
      pingSent = false;
      break;
    case Opcode_t::CLOSE:
      spdlog::debug("WebSocket received close");
      // Echo the close back
      addTransmitBuffer(frame.toBuffer());
      return ResultCode_t::SUCCESS;
  }

  // Frame is done, do something
  frame = Frame();

  // If the receive buffer has more data (multiple frames in the buffer),
  // recursively process them
  if (length != 0) {
    return processReceiveBuffer(begin, length);
  }

  return ResultCode_t::INCOMPLETE;
}

/**
 * @brief Check the completion of the protocol
 *
 * @return true if a message is being processed or waiting for a message
 * @return false if all messages have been processed and no more are expected
 */
bool WebSocket::isDone() {
  return frame.getOpcode() == Opcode_t::CLOSE && AppProtocol::isDone();
}

/**
 * @brief Send a check to test the connection for aliveness
 *
 * @return true when the protocol has already sent an alive check
 * @return false when the protocol has not sent an alive check yet
 */
bool WebSocket::sendAliveCheck() {
  if (pingSent)
    return true;
  // send ping
  frame = Frame();
  frame.setOpcode(Opcode_t::PING);
  frame.addData("Ping");
  addTransmitBuffer(frame.toBuffer());
  pingSent = true;
  return false;
}

} // namespace WebSocket
} // namespace Web