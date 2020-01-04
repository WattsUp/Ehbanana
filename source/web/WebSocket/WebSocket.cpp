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
WebSocket::~WebSocket() {}

/**
 * @brief Process a received buffer, could be the entire message or a fragment
 *
 * @param begin character
 * @param length of buffer
 * @throw std::exception Thrown on failure
 */
void WebSocket::processReceiveBuffer(const uint8_t * begin, size_t length) {
  if (!frameIn.decode(begin, length)) // TODO pass in file buffer for if binary
    return;                           // More data for the frame

  switch (frameIn.getOpcode()) {
    case Opcode_t::TEXT:
      processFrameText();
      break;
    case Opcode_t::BINARY:
      // TODO close current file buffer as it is done loading transferring
      break;
    case Opcode_t::PING: {
      debug("WebSocket received ping");
      // Send pong
      std::shared_ptr<Frame> frame = std::make_shared<Frame>();
      frame->addData(frameIn.getData());
      frame->setOpcode(Opcode_t::PONG);
      framesOut.push_front(frame);
    } break;
    case Opcode_t::PONG:
      debug("WebSocket received pong");
      pingSent = false;
      break;
    case Opcode_t::CLOSE:
      debug("WebSocket received close");
      // Echo the close back
      std::shared_ptr<Frame> frame = std::make_shared<Frame>();
      frame->addData(frameIn.getData());
      frame->setOpcode(Opcode_t::CLOSE);
      framesOut.push_front(frame);
      pingSent = true;
  }

  // Frame is done, clear
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

  if (doc.HasMember("fileSize")) {
    doc["fileSize"].GetInt(); // TODO call file callback
    // Create a file buffer
    // Pass it to frames until file is done loading
  }
  doc["href"].GetString();
  doc["id"].GetString();
  doc["value"].GetString();
  // TODO call input callback
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
  // Send ping
  std::shared_ptr<Frame> frame = std::make_shared<Frame>();
  frame->setOpcode(Opcode_t::PING);
  frame->addData("Ping");
  framesOut.push_back(frame);
  pingSent = true;
  return false;
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
    currentFrameOut = framesOut.front();
    framesOut.pop_front();
    addTransmitBuffer(currentFrameOut->getBuffers());
    return true;
  }
  return false;
}

} // namespace WebSocket
} // namespace Web
} // namespace Ehbanana