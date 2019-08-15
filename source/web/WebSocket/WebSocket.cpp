#include "WebSocket.h"

#include <rapidjson/document.h>
#include <spdlog/spdlog.h>

namespace Web {
namespace WebSocket {

/**
 * @brief Construct a new WebSocket::WebSocket object
 *
 * @param gui that owns this server
 */
WebSocket::WebSocket(EBGUI_t gui) : gui(gui) {}

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
      result = processFrameText();
      if (!result)
        return result;
      break;
    case Opcode_t::BINARY:
      fseek(frame.getDataFile(), 0L, SEEK_END);
      spdlog::debug("WebSocket received binary of length: {}",
          ftell(frame.getDataFile()));
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
 * @brief Process the current frame as text by parsing the JSON and enqueuing a
 * message
 *
 * @return Result
 */
Result WebSocket::processFrameText() {
  EBMessage_t msg;
  msg.gui = gui;
  msg.type = EBMSGType_t::INPUT;
  // Parse JSON
  rapidjson::Document d;
  if (d.Parse(frame.getData().c_str()).HasParseError())
    return ResultCode_t::READ_FAULT + frame.getData() + "Parsing JSON";

  // Populate message
  rapidjson::Value::MemberIterator i = d.FindMember("name");
  if (i == d.MemberEnd() || !i->value.IsString())
    return ResultCode_t::INVALID_DATA + frame.getData() + "No \"name\"";
  msg.htmlID.add(i->value.GetString());
  i = d.FindMember("value");
  if (i == d.MemberEnd() || !i->value.IsString())
    return ResultCode_t::INVALID_DATA + frame.getData() + "No \"value\"";
  msg.htmlValue.add(i->value.GetString());
  i = d.FindMember("checked");
  if (i != d.MemberEnd()) {
    if (!i->value.IsBool())
      return ResultCode_t::INVALID_DATA + frame.getData() +
             "\"checked\" is not a bool";
    msg.checked.add(i->value.GetBool() ? "true" : "false");
  }
  EBEnqueueMessage(msg);
  return ResultCode_t::SUCCESS;
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