#include "WebSocket.h"

#include "EhbananaLog.h"

#include <rapidjson/document.h>

namespace Ehbanana {
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
WebSocket::~WebSocket() {
  for (Frame * frame : framesOut)
    delete frame;
}

/**
 * @brief Process a received buffer, could be the entire message or a fragment
 *
 * @param begin character
 * @param length of buffer
 * @return Result error code
 */
Result WebSocket::processReceiveBuffer(const uint8_t * begin, size_t length) {
  Result result = frameIn.decode(begin, length);
  if (!result)
    return result;

  switch (frameIn.getOpcode()) {
    case Opcode_t::TEXT:
      result = processFrameText();
      if (!result)
        return result;
      break;
    case Opcode_t::BINARY:
      result = processFrameBinary();
      if (!result)
        return result;
      break;
    case Opcode_t::PING: {
      debug("WebSocket received ping");
      // Send pong
      Frame * frame = new Frame();
      frame->addData(frameIn.getData());
      frame->setOpcode(Opcode_t::PONG);
      framesOut.push_back(frame);
      return ResultCode_t::INCOMPLETE;
    }
    case Opcode_t::PONG:
      debug("WebSocket received pong");
      pingSent = false;
      break;
    case Opcode_t::CLOSE:
      debug("WebSocket received close");
      // Echo the close back
      Frame * frame = new Frame();
      frame->addData(frameIn.getData());
      frame->setOpcode(Opcode_t::CLOSE);
      framesOut.push_back(frame);
      addTransmitBuffer(frameIn.toBuffer());
      return ResultCode_t::SUCCESS;
  }

  // Frame is done, do something
  frameIn = Frame();

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
  msg.gui  = gui;
  msg.type = EBMSGType_t::INPUT;
  rapidjson::Document doc;

  // Parse JSON
  if (doc.Parse(frameIn.getData().c_str()).HasParseError())
    return ResultCode_t::READ_FAULT + frameIn.getData() + "Parsing JSON";

  auto i = doc.FindMember("href");
  if (i == doc.MemberEnd() || !i->value.IsString())
    return ResultCode_t::INVALID_DATA + "No 'href'";
  msg.href.add(i->value.GetString());

  i = doc.FindMember("id");
  if (i == doc.MemberEnd() || !i->value.IsString())
    return ResultCode_t::INVALID_DATA + "No 'id'";
  msg.id.add(i->value.GetString());

  i = doc.FindMember("value");
  if (i == doc.MemberEnd() || !i->value.IsString())
    return ResultCode_t::INVALID_DATA + "No 'value'";
  msg.value.add(i->value.GetString());

  i = doc.FindMember("fileSize");
  if (i != doc.MemberEnd()) {
    if (!i->value.IsInt())
      return ResultCode_t::INVALID_DATA + frameIn.getData() +
             "\"fileSize\" is not an int";
    msg.fileSize    = i->value.GetInt();
    msgAwaitingFile = msg;
    return ResultCode_t::SUCCESS;
  }

  EBEnqueueMessage(msg);
  return ResultCode_t::SUCCESS;
}

/**
 * @brief Process the current frame as binary by parsing the JSON and enqueuing
 * a message. Adds the received file to the preceeding message and enqueues it
 *
 * @return Result
 */
Result WebSocket::processFrameBinary() {
  fseek(frameIn.getDataFile(), 0L, SEEK_END);
  if (msgAwaitingFile.fileSize != ftell(frameIn.getDataFile())) {
    return ResultCode_t::INVALID_DATA +
           "Received file's size does not match preceeding message's";
  }
  rewind(frameIn.getDataFile());
  msgAwaitingFile.file = frameIn.getDataFile(true);
  EBEnqueueMessage(msgAwaitingFile);
  return ResultCode_t::SUCCESS;
}

/**
 * @brief Check the completion of the protocol
 *
 * @return true if a message is being processed or waiting for a message
 * @return false if all messages have been processed and no more are expected
 */
bool WebSocket::isDone() {
  return frameIn.getOpcode() == Opcode_t::CLOSE && AppProtocol::isDone();
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
  Frame * frame = new Frame();
  frame->setOpcode(Opcode_t::PING);
  frame->addData("Ping");
  framesOut.push_back(frame);
  pingSent = true;
  return false;
}

/**
 * @brief Add a message to transmit out if available
 * returns ResultCode_t::NOT_SUPPORTED if not compatible
 *
 * @param msg to add
 * @return Result
 */
Result WebSocket::addMessage(const std::string & msg) {
  Frame * frame = new Frame();
  frame->setOpcode(Opcode_t::TEXT);
  frame->addData(msg);
  framesOut.push_back(frame);
  return ResultCode_t::SUCCESS;
}

/**
 * @brief Update the transmit buffers with number of bytes transmitted
 * Removes buffers that have been completely transmitted. Moves the start
 * pointer of the next buffer that has not been transmitted.
 *
 * @param bytesWritten
 * @return true when all transmit buffers have been transmitted
 * @return false when there are more transmit buffers
 */
bool WebSocket::updateTransmitBuffers(size_t bytesWritten) {
  bool result = AppProtocol::updateTransmitBuffers(bytesWritten);
  if (result) {
    // Remove the current frame;
    delete framesOut.front();
    framesOut.pop_front();
  }
  return result;
}

/**
 * @brief Check if there are buffers in the transmit queue that have not been
 * transmitted
 *
 * @return true when the transmit buffers are not empty
 * @return false when the transmit buffers are empty
 */
bool WebSocket::hasTransmitBuffers() {
  if (AppProtocol::hasTransmitBuffers())
    return true;
  if (!framesOut.empty()) {
    addTransmitBuffer(framesOut.front()->toBuffer());
    return true;
  }
  return false;
}

} // namespace WebSocket
} // namespace Web
} // namespace Ehbanana