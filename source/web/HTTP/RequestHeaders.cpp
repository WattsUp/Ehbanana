#include "RequestHeaders.h"

#include "EhbananaLog.h"

namespace Ehbanana {
namespace Web {
namespace HTTP {

/**
 * @brief Construct a new Request Headers:: Request Headers object
 *
 */
RequestHeaders::RequestHeaders() {}

/**
 * @brief Add a header to the list of headers, validate the header and set
 * fields if appropriate
 *
 * @param header to add
 * @throw std::exception Thrown on failure
 */
void RequestHeaders::addHeader(HeaderHash_t header) {
  switch (header.name.get()) {
    case Hash::calculateHash("Content-Length"):
      contentLength = static_cast<size_t>(std::stoll(header.value.getString()));
      break;
    case Hash::calculateHash("Connection"):
      addConnection(header);
      break;
    case Hash::calculateHash("Upgrade"):
      addUpgrade(header);
      break;
    case Hash::calculateHash("Sec-WebSocket-Key"):
      webSocketKey = header.value.getString();
      break;
    case Hash::calculateHash("Sec-WebSocket-Version"):
      webSocketVersion = header.value;
      break;
    case Hash::calculateHash("Host"):
    case Hash::calculateHash("Upgrade-Insecure-Requests"):
    case Hash::calculateHash("User-Agent"):
    case Hash::calculateHash("Accept"):
    case Hash::calculateHash("Accept-Encoding"):
    case Hash::calculateHash("Accept-Language"):
    case Hash::calculateHash("Referer"):
    case Hash::calculateHash("Cache-Control"):
    case Hash::calculateHash("Pragma"):
    case Hash::calculateHash("Origin"):
    case Hash::calculateHash("Sec-WebSocket-Extensions"):
    case Hash::calculateHash("Sec-Fetch-Mode"):
    case Hash::calculateHash("Sec-Fetch-User"):
    case Hash::calculateHash("Sec-Fetch-Site"):
    case Hash::calculateHash("Content-Type"):
      // Ignoring
      break;
    default:
      warn("The request header is not recognized");
  }
}

/**
 * @brief Add connection header
 *
 * @param header to add
 * @throw std::exception Thrown on failure
 */
void RequestHeaders::addConnection(HeaderHash_t header) {
  switch (header.value.get()) {
    case Hash::calculateHash("close"):
      connection = Connection_t::CLOSE;
      break;
    case Hash::calculateHash("Keep-Alive"):
    case Hash::calculateHash("keep-alive"):
      connection = Connection_t::KEEP_ALIVE;
      break;
    case Hash::calculateHash("Upgrade"):
      connection = Connection_t::UPGRADE;
      break;
    default:
      throw std::exception("Unknown connection header value");
  }
}

/**
 * @brief Add upgrade header
 *
 * @param header to add
 * @throw std::exception Thrown on failure
 */
void RequestHeaders::addUpgrade(HeaderHash_t header) {
  switch (header.value.get()) {
    case Hash::calculateHash("websocket"):
      upgrade = Upgrade_t::WEB_SOCKET;
      break;
    default:
      throw std::exception("Unknown upgrade header value");
  }
}

/**
 * @brief Get the content length header value, 0 if not set
 *
 * @return const size_t
 */
const size_t RequestHeaders::getContentLength() const {
  return contentLength;
}

/**
 * @brief Get the connection header, CLOSE if not set
 *
 * @return const RequestHeaders::Connection_t
 */
const RequestHeaders::Connection_t RequestHeaders::getConnection() const {
  return connection;
}

/**
 * @brief Get the requested upgrade protocol
 *
 * @return const Upgrade_t
 */
const RequestHeaders::Upgrade_t RequestHeaders::getUpgrade() const {
  return upgrade;
}

/**
 * @brief Get the web socket key
 *
 * @return const HashValue_t
 */
const std::string & RequestHeaders::getWebSocketKey() const {
  return webSocketKey;
}

/**
 * @brief Get the web socket version
 *
 * @return const HashValue_t
 */
const HashValue_t RequestHeaders::getWebSocketVersionHash() const {
  return webSocketVersion.get();
}

} // namespace HTTP
} // namespace Web
} // namespace Ehbanana