#include "WebSocket.h"

#include "EhbananaLog.h"

#include <rapidjson/document.h>

namespace Ehbanana {
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
WebSocket::~WebSocket() {
  for (Frame * frame : framesOut)
    delete frame;
}

/**
 * @brief Process a received buffer, could be the entire message or a fragment
 *
 * @param begin character
 * @param length of buffer
 * @throw std::exception Thrown on failure
 */
void WebSocket::processReceiveBuffer(const uint8_t * begin, size_t length) {
  frameIn.decode(begin, length);

  switch (frameIn.getOpcode()) {
    case Opcode_t::TEXT:
      processFrameText();
      break;
    case Opcode_t::BINARY:
      processFrameBinary();
      break;
    case Opcode_t::PING: {
      debug("WebSocket received ping");
      // Send pong
      Frame * frame = new Frame();
      frame->addData(frameIn.getData());
      frame->setOpcode(Opcode_t::PONG);
      framesOut.push_back(frame);
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
  }

  // Frame is done, do something
  frameIn = Frame();

  // If the receive buffer has more data (multiple frames in the buffer),
  // recursively process them
  if (length != 0) {
    processReceiveBuffer(begin, length);
  }
}

/**
 * @brief Process the current frame as text by parsing the JSON and enqueuing a
 * message
 *
 * @throw std::exception Thrown on failure
 */
void WebSocket::processFrameText() {
  rapidjson::Document doc;

  // Parse JSON
  if (doc.Parse(frameIn.getData().c_str()).HasParseError())
    throw std::exception("Failed to parse JSON");

  auto i = doc.FindMember("href");
  if (i == doc.MemberEnd() || !i->value.IsString())
    throw std::exception("No href");
  // msg.href.add(i->value.GetString());

  // i = doc.FindMember("id");
  // if (i == doc.MemberEnd() || !i->value.IsString())
  //   return ResultCode_t::INVALID_DATA + "No 'id'";
  // msg.id.add(i->value.GetString());

  // i = doc.FindMember("value");
  // if (i == doc.MemberEnd() || !i->value.IsString())
  //   return ResultCode_t::INVALID_DATA + "No 'value'";
  // msg.value.add(i->value.GetString());

  // i = doc.FindMember("fileSize");
  // if (i != doc.MemberEnd()) {
  //   if (!i->value.IsInt())
  //     return ResultCode_t::INVALID_DATA + frameIn.getData() +
  //            "\"fileSize\" is not an int";
  //   msg.fileSize    = i->value.GetInt();
  //   msgAwaitingFile = msg;
  //   return ResultCode_t::SUCCESS;
  // }

  // EBEnqueueMessage(msg);
}

/**
 * @brief Process the current frame as binary by parsing the JSON and enqueuing
 * a message. Adds the received file to the preceeding message and enqueues it
 *
 * @throw std::exception Thrown on failure
 */
void WebSocket::processFrameBinary() {
  // fseek(frameIn.getDataFile(), 0L, SEEK_END);
  // if (msgAwaitingFile.fileSize != (size_t)ftell(frameIn.getDataFile())) {
  //   return ResultCode_t::INVALID_DATA +
  //          "Received file's size does not match preceeding message's";
  // }
  // rewind(frameIn.getDataFile());
  // msgAwaitingFile.file = frameIn.getDataFile(true);
  // EBEnqueueMessage(msgAwaitingFile);
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
 * @throw std::exception Thrown on failure
 */
void WebSocket::addMessage(const std::string & msg) {
  Frame * frame = new Frame();
  frame->setOpcode(Opcode_t::TEXT);
  frame->addData(msg);
  framesOut.push_back(frame);
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