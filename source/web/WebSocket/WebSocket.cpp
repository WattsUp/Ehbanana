#include "WebSocket.h"

#include "../Server.h"
#include "EhbananaLog.h"

#include <rapidjson/document.h>

namespace Ehbanana {
namespace Web {
namespace WebSocket {

/**
 * @brief Construct a new WebSocket::WebSocket object
 *
 * @param server parent to callback
 */
WebSocket::WebSocket(Server * server) : AppProtocol(server) {}

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
  if (!frameIn.decode(begin, length))
    return; // More data for the frame

  switch (frameIn.getOpcode()) {
    case Opcode_t::TEXT:
      processFrameText();
      break;
    case Opcode_t::BINARY:
      throw std::exception("WebSocket binary is not supported");
      break;
    case Opcode_t::PING: {
      debug("WebSocket received ping");
      // Send pong
      std::shared_ptr<Frame> frame = std::make_shared<Frame>();
      frame->addData(frameIn.getString());
      frame->setOpcode(Opcode_t::PONG);
      framesOut.push_front(frame);
    } break;
    case Opcode_t::PONG:
      debug("WebSocket received pong");
      pingSent = false;
      // Frame is done, clear
      frameIn = Frame();
      break;
    case Opcode_t::CLOSE:
      debug("WebSocket received close");
      // Echo the close back
      std::shared_ptr<Frame> frame = std::make_shared<Frame>();
      frame->addData(frameIn.getString());
      frame->setOpcode(Opcode_t::CLOSE);
      framesOut.push_front(frame);
      pingSent = true;
  }

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
  if (doc.Parse(frameIn.getString().c_str()).HasParseError())
    throw std::exception("Failed to parse JSON");

  if (!doc.HasMember("href") || !doc["href"].IsString())
    throw std::exception("WebSocket input href is invalid");
  if (!doc.HasMember("id") || !doc["id"].IsString())
    throw std::exception("WebSocket input id is invalid");
  if (!doc.HasMember("value") || !doc["href"].IsString())
    throw std::exception("WebSocket input value is invalid");

  server->enqueueCallback(
      doc["href"].GetString(), doc["id"].GetString(), doc["value"].GetString());

  // Frame is done, clear
  frameIn = Frame();
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
 * @brief Enqueue an output message
 *
 * @param message
 */
void WebSocket::enqueueOutput(std::shared_ptr<Ehbanana::MessageOut> message) {
  std::shared_ptr<Frame> frame = std::make_shared<Frame>(message);
  frame->setOpcode(Opcode_t::TEXT);
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