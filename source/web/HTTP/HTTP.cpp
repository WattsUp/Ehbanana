#include "HTTP.h"

#include "../Server.h"
#include "CacheControl.h"
#include "EhbananaLog.h"
#include "Hash.h"
#include "MIMETypes.h"
#include "Utils.h"

#include <algorithm/sha1.hpp>
#include <base64.h>
#include <memory>
#include <sys/stat.h>
#include <sys/types.h>

namespace Ehbanana {
namespace Web {
namespace HTTP {

/**
 * @brief Construct a new HTTP::HTTP object
 *
 * @param server parent to callback
 */
HTTP::HTTP(Server * server) : AppProtocol(server) {}

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
 * @throw std::exception Thrown on failure
 */
void HTTP::processReceiveBuffer(const uint8_t * begin, size_t length) {
  switch (state) {
    case State_t::READING_HEADER:
      request.parse(begin, begin + length);
      if (!request.isHeaderParsed())
        break; // Get more data
      // Header is parsed, process request, body may not be done
      try {
        handleRequest();
      } catch (const std::exception & e) {
        log(EBLogLevel_t::EB_WARNING,
            "Exception occurred whilst handling HTTP request: %s", e.what());
        reply = Reply::stockReply(Status_t::INTERNAL_SERVER_ERROR);
      }

      if (!request.isBodyParsed()) {
        state = State_t::READING_BODY;
        break;
      } else {
        state = State_t::READING_DONE;
        processReceiveBuffer(begin, 0); // jump to reading done
      }
      break;
    case State_t::READING_BODY:
      request.parse(begin, begin + length);
      if (!request.isBodyParsed())
        break; // Get more data

      state = State_t::READING_DONE;
      // Fall through
    case State_t::READING_DONE:
      addTransmitBuffer(reply.getBuffers());
      state = State_t::WRITING;
      break;
    case State_t::WRITING:
    default:
      throw std::exception(("Invalid HTTP state during RX: " +
                            std::to_string(static_cast<uint8_t>(state)))
                               .c_str());
  }
}

/**
 * @brief Check the completion of the protocol
 *
 * @return true if all messages have been processed and no more are expected
 * @return false if a message is being processed or waiting for a message
 */
bool HTTP::isDone() {
  if (AppProtocol::isDone()) {
    // No output messages waiting to transmit
    switch (state) {
      case State_t::READING_HEADER:
      case State_t::READING_BODY:
      case State_t::READING_DONE:
        return false;
      case State_t::WRITING:
        if (request.getHeaders().getConnection() ==
            RequestHeaders::Connection_t::KEEP_ALIVE) {
          request = Request();
          reply   = Reply();
          state   = State_t::READING_HEADER;
          return false; // Writing completed, keep alive
        } else
          return true; // Writing completed, don't keep alive
      default:
        throw std::exception(("Invalid HTTP state during isDone: " +
                              std::to_string(static_cast<uint8_t>(state)))
                                 .c_str());
    }
  } else
    return false;
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
 * @throw std::exception Thrown on failure
 */
void HTTP::handleRequest() {
  switch (request.getMethodHash()) {
    case Hash::calculateHash("GET"):
      if (request.getHeaders().getConnection() ==
          RequestHeaders::Connection_t::UPGRADE) {
        handleUpgrade();
      } else {
        handleGET();
      }
      break;
    case Hash::calculateHash("POST"):
      handlePOST();
      break;
    default:
      throw std::exception("Unknown HTTP request method");
  }
}

/**
 * @brief Handle the GET request and populate the reply
 *
 * @throw std::exception Thrown on failure
 */
void HTTP::handleGET() {
  std::string uri = request.getURI();
  if (request.getQueries().empty())
    info("GET URI: \"" + uri + "\"");
  else {
    std::string buffer = "";
    for (const Request::Query_t & query : request.getQueries()) {
      buffer += "\n    \"" + query.name + "\"=\"" + query.value + "\"";
    }
    info("GET URI: \"" + uri + "\" Queries:" + buffer);
  }

  // URI must be absolute
  if (uri.empty() || uri[0] != '/' || uri.find("..") != std::string::npos) {
    warn("URI is not absolute: " + uri);
    reply = Reply::stockReply(Status_t::BAD_REQUEST);
    return;
  }

  uri = root() + uri;

  // URI ends in forward slash, load its index.html
  if (uri[uri.size() - 1] == '/') {
    uri += "index.html";
  }

  struct stat info;
  if (stat(uri.c_str(), &info) != 0) {
    // TODO call outputFileCallback first, if still not found, 404
    reply = Reply::stockReply(Status_t::NOT_FOUND);
    return;
  }

  // URI is directory, load its index.html
  if (info.st_mode & S_IFDIR)
    uri += "/index.html";

  std::shared_ptr<MemoryMapped> file =
      std::make_shared<MemoryMapped>(uri, 0, MemoryMapped::SequentialScan);
  if (!file->isValid()) {
    reply = Reply::stockReply(Status_t::INTERNAL_SERVER_ERROR);
    return;
  }

  // Determine the file extension.
  reply.addHeader("Content-Type", MIMETypes::Instance()->getType(uri));
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
}

/**
 * @brief Handle the POST request and populate the reply
 *
 * @throw std::exception Thrown on failure
 */
void HTTP::handlePOST() {
  std::string uri = request.getURI();
  if (request.getQueries().empty())
    info("POST URI: \"" + uri + "\"");
  else {
    std::string buffer = "";

    std::string id;
    std::string value;
    for (const Request::Query_t & query : request.getQueries()) {
      if (query.name.compare("eb-file-id") == 0)
        id = query.value;
      if (query.name.compare("eb-file-value") == 0)
        value = query.value;
      buffer += "\n    \"" + query.name + "\"=\"" + query.value + "\"";
    }
    info("POST URI: \"" + uri + "\" Queries:" + buffer);

    if (!id.empty() && !value.empty()) {
      // File upload
      server->enqueueCallback(uri, id, value, request.getBody());
      reply.setStatus(Status_t::OK);
      return;
    }
    // else POST not from ehbanana
  }

  reply.setStatus(Status_t::NOT_IMPLEMENTED);
}

/**
 * @brief Handle a request to upgrade the connection
 *
 * @param request to handle
 * @param reply to populate
 * @throw std::exception Thrown on failure
 */
void HTTP::handleUpgrade() {
  if (request.getHeaders().getUpgrade() !=
      RequestHeaders::Upgrade_t::WEB_SOCKET) {
    warn("Only upgrading to web socket is supported");
    reply = Reply::stockReply(Status_t::NOT_IMPLEMENTED);
    return;
  }

  reply.setStatus(Status_t::SWITCHING_PROTOCOLS);
  if (request.getHeaders().getWebSocketVersionHash() !=
      Hash::calculateHash("13")) {
    warn("Only upgrading to web socket version 13 is supported");
    reply = Reply::stockReply(Status_t::NOT_IMPLEMENTED);
    return;
  }

  reply.addHeader("Upgrade", "websocket");
  reply.addHeader("Connection", "Upgrade");

  // Perform SHA 1 with the magic string
  digestpp::sha1 sha;
  sha.absorb(request.getHeaders().getWebSocketKey());
  sha.absorb("258EAFA5-E914-47DA-95CA-C5AB0DC85B11");
  uint8_t buf[20];
  sha.digest(buf, 20);
  std::string shaBase64 = base64_encode(buf, 20);
  reply.addHeader("Sec-WebSocket-Accept", shaBase64);
}

} // namespace HTTP
} // namespace Web
} // namespace Ehbanana