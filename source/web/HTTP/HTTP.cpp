#include "HTTP.h"

#include "CacheControl.h"
#include "EhbananaLog.h"
#include "MIMETypes.h"

#include <algorithm/sha1.hpp>
#include <base64.h>

namespace Ehbanana {
namespace Web {
namespace HTTP {

/**
 * @brief Construct a new HTTP::HTTP object
 *
 */
HTTP::HTTP() {}

/**
 * @brief Destroy the HTTP::HTTP object
 *
 */
HTTP::~HTTP() {}

/**
 * @brief Process a received buffer, could be the entire message or a fragment
 *
 * @param begin character
 * @param length of buffer
 * @return Result error code
 */
Result HTTP::processReceiveBuffer(const uint8_t * begin, size_t length) {
  Result result;
  switch (state) {
    case State_t::READING:
      result = request.parse(begin, begin + length);
      if (!result)
        return result;
      state = State_t::READING_DONE;
      // Fall through
    case State_t::READING_DONE:
      // Handle request
      result = handleRequest();
      if (!result) {
        warn((result + "Handling HTTP request").getMessage());
        reply = Reply::stockReply(result);
      }
      addTransmitBuffer(reply.getBuffers());
      state = State_t::WRITING;
      return ResultCode_t::INCOMPLETE;
    case State_t::WRITING:
    case State_t::WRITING_DONE:
    case State_t::COMPLETE:
    default:
      return ResultCode_t::INVALID_STATE +
             ("HTTP: " + std::to_string(static_cast<uint8_t>(state)));
  }
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
bool HTTP::updateTransmitBuffers(size_t bytesWritten) {
  bool done = AppProtocol::updateTransmitBuffers(bytesWritten);
  if (done) {
    if (request.getHeaders().getConnection() ==
        RequestHeaders::Connection_t::KEEP_ALIVE) {
      request = Request();
      reply   = Reply();
      state   = State_t::READING;
    } else
      state = State_t::COMPLETE;
  }
  return done;
}

/**
 * @brief Check the completion of the protocol
 *
 * @return true if a message is being processed or waiting for a message
 * @return false if all messages have been processed and no more are expected
 */
bool HTTP::isDone() {
  return state == State_t::COMPLETE;
}

/**
 * @brief Get the requested protocol to change to
 *
 * @return AppProtocol_t protocol requested, NONE for no change requested
 */
AppProtocol_t HTTP::getChangeRequest() {
  if (request.getHeaders().getConnection() ==
      RequestHeaders::Connection_t::UPGRADE) {
    if (request.getHeaders().getUpgrade() ==
        RequestHeaders::Upgrade_t::WEB_SOCKET)
      return AppProtocol_t::WEBSOCKET;
  }
  return AppProtocol_t::NONE;
}

/**
 * @brief Handle the request and populate the reply
 *
 * @return Result error code
 */
Result HTTP::handleRequest() {
  Result result;
  switch (request.getMethod().get()) {
    case Hash::calculateHash("GET"):
      if (request.getHeaders().getConnection() ==
          RequestHeaders::Connection_t::UPGRADE) {
        result = handleUpgrade();
        if (!result)
          return result + "Handling Connection: Upgrade";
      } else {
        result = handleGET();
        if (!result)
          return result + "Handling GET";
      }
      break;
    case Hash::calculateHash("POST"):
      result = handlePOST();
      if (!result)
        return result + "Handling POST";
      break;
    default:
      return ResultCode_t::UNKNOWN_HASH +
             ("Request method: " + request.getMethod().getString());
  }
  return ResultCode_t::SUCCESS;
}

/**
 * @brief Handle the GET request and populate the reply
 *
 * @return Result error code
 */
Result HTTP::handleGET() {
  std::string uri = request.getURI().getString();
  if (request.getQueries().empty())
    info("GET URI: \"" + uri + "\"");
  else {
    std::string buffer = "";
    for (HeaderHash_t query : request.getQueries()) {
      buffer += "\n    \"" + query.name.getString() + "\"=\"" +
                query.value.getString() + "\"";
    }
    info("GET URI: \"" + uri + "\" Queries:" + buffer);
  }

  // URI must be absolute
  if (uri.empty() || uri[0] != '/' || uri.find("..") != std::string::npos)
    return ResultCode_t::INVALID_DATA + ("URI is not absolute: " + uri);

  // Add index.html to folders
  if (uri[uri.size() - 1] == '/')
    uri += "index.html";

  // Determine the file extension.
  reply.addHeader("Content-Type", MIMETypes::Instance()->getType(uri));
  MemoryMapped * file =
      new MemoryMapped(root() + uri, 0, MemoryMapped::SequentialScan);
  if (!file->isValid()) {
    file->close();
    delete file;
    return ResultCode_t::OPEN_FAILED + (root() + uri);
  }
  reply.addHeader("Content-Length", std::to_string(file->size()));
  // TODO change cache control to any type of header that match the regex
  reply.addHeader(
      "Cache-Control", CacheControl::Instance()->getCacheControl(uri));
  switch (request.getHeaders().getConnection()) {
    default:
    case RequestHeaders::Connection_t::CLOSE:
      reply.addHeader("Connection", "close");
      break;
    case RequestHeaders::Connection_t::KEEP_ALIVE:
      reply.addHeader("Connection", "keep-alive");
      break;
  }
  reply.setContent(file);

  return ResultCode_t::SUCCESS;
}

/**
 * @brief Handle the POST request and populate the reply
 *
 * @return Result error code
 */
Result HTTP::handlePOST() {
  if (request.getQueries().empty())
    info("POST URI: \"" + request.getURI().getString() + "\"");
  else {
    std::string buffer = "";
    for (HeaderHash_t query : request.getQueries()) {
      buffer += "\n    \"" + query.name.getString() + "\"=\"" +
                query.value.getString() + "\"";
    }
    info("POST URI: \"" + request.getURI().getString() +
         "\" Queries: " + buffer);
  }

  reply.setStatus(Status_t::NOT_IMPLEMENTED);

  return ResultCode_t::NOT_SUPPORTED + "handlePOST";
}

/**
 * @brief Handle a request to upgrade the connection
 *
 * @param request to handle
 * @param reply to populate
 * @return Result error code
 */
Result HTTP::handleUpgrade() {
  if (request.getHeaders().getUpgrade() !=
      RequestHeaders::Upgrade_t::WEB_SOCKET)
    return ResultCode_t::NOT_SUPPORTED +
           ("HTTP upgrade to: " + std::to_string(static_cast<uint8_t>(
                                      request.getHeaders().getUpgrade())));

  reply.setStatus(Status_t::SWITCHING_PROTOCOLS);
  if (request.getHeaders().getWebSocketVersion().get() !=
      Hash::calculateHash("13"))
    return ResultCode_t::BAD_COMMAND +
           ("Websocket version" +
               request.getHeaders().getWebSocketVersion().getString());

  reply.addHeader("Upgrade", "websocket");
  reply.addHeader("Connection", "Upgrade");

  // Perform SHA 1 with the magic string
  digestpp::sha1 sha;
  sha.absorb(request.getHeaders().getWebSocketKey().getString());
  sha.absorb("258EAFA5-E914-47DA-95CA-C5AB0DC85B11");
  uint8_t buf[20];
  sha.digest(buf, 20);
  std::string shaBase64 = base64_encode(buf, 20);
  reply.addHeader("Sec-WebSocket-Accept", shaBase64);

  return ResultCode_t::SUCCESS;
}

} // namespace HTTP
} // namespace Web
} // namespace Ehbanana