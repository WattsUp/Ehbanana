#include "WebSocket.h"

#include <rapidjson/document.h>
#include <rapidjson/prettywriter.h>
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
      spdlog::debug("WebSocket received ping");
      // Send pong
      Frame * frame = new Frame();
      frame->addData(frameIn.getData());
      frame->setOpcode(Opcode_t::PONG);
      framesOut.push_back(frame);
      return ResultCode_t::INCOMPLETE;
    }
    case Opcode_t::PONG:
      spdlog::debug("WebSocket received pong");
      pingSent = false;
      break;
    case Opcode_t::CLOSE:
      spdlog::debug("WebSocket received close");
      // Echo the close back
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
  // Parse JSON
  rapidjson::Document doc;
  if (doc.Parse(frameIn.getData().c_str()).HasParseError())
    return ResultCode_t::READ_FAULT + frameIn.getData() + "Parsing JSON";

  // Populate message
  rapidjson::Value::MemberIterator i = doc.FindMember("name");
  if (i == doc.MemberEnd() || !i->value.IsString())
    return ResultCode_t::INVALID_DATA + frameIn.getData() + "No \"name\"";
  msg.htmlID.add(i->value.GetString());
  i = doc.FindMember("value");
  if (i == doc.MemberEnd() || !i->value.IsString())
    return ResultCode_t::INVALID_DATA + frameIn.getData() + "No \"value\"";
  msg.htmlValue.add(i->value.GetString());
  i = doc.FindMember("checked");
  if (i != doc.MemberEnd()) {
    if (!i->value.IsBool())
      return ResultCode_t::INVALID_DATA + frameIn.getData() +
             "\"checked\" is not a bool";
    msg.checked.add(i->value.GetBool() ? "true" : "false");
  }
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
Result WebSocket::addMessage(const EBMessage_t & msg) {
  rapidjson::Document doc;
  doc.SetObject();

  rapidjson::Value value;
  value = rapidjson::StringRef(msg.htmlID.getString().c_str());
  doc.AddMember("name", value, doc.GetAllocator());

  value = rapidjson::StringRef(msg.htmlValue.getString().c_str());
  doc.AddMember("value", value, doc.GetAllocator());

  value = rapidjson::StringRef(msg.checked.getString().c_str());
  doc.AddMember("checked", value, doc.GetAllocator());

  rapidjson::StringBuffer                          sb;
  rapidjson::PrettyWriter<rapidjson::StringBuffer> writer(sb);
  doc.Accept(writer);
  Frame * frame = new Frame();
  frame->setOpcode(Opcode_t::TEXT);
  frame->addData(sb.GetString());
  framesOut.push_back(frame);
  return ResultCode_t::SUCCESS;
}

/**
 * @brief Check if there are buffers in the transmit queue that have not been
 * transmitted
 *
 * @return true when the transmit buffers are not empty
 * @return false when the transmit buffers are empty
 */
bool WebSocket::hasTransmitBuffers() {
  if(AppProtocol::hasTransmitBuffers())
    return true;
  if(!framesOut.empty()){
    addTransmitBuffer(framesOut.front()->toBuffer());
    framesOut.pop_front();
    return true;
  }
  return false;
}

} // namespace WebSocket
} // namespace Web