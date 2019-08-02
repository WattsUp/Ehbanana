#include "RequestHeaders.h"

namespace Web {

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
 * @return Result error code
 */
Result RequestHeaders::addHeader(HeaderHash_t header) {
  Result result;
  switch (header.name.get()) {
    case Hash::calculateHash("Content-Length"):
      contentLength = static_cast<size_t>(std::stoll(header.value.getString()));
      break;
    case Hash::calculateHash("Connection"):
      return addConnection(header);
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
      // Ignoring
      break;
    default:
      return ResultCode_t::UNKNOWN_HASH +
             ("Request header: " + header.name.getString());
  }
  return ResultCode_t::SUCCESS;
}

/**
 * @brief Add connection header
 *
 * @param header to add
 * @return Result error code
 */
Result RequestHeaders::addConnection(HeaderHash_t header) {
  switch (header.value.get()) {
    case Hash::calculateHash("close"):
      connection = Connection::CLOSE;
      break;
    case Hash::calculateHash("keep-alive"):
      connection = Connection::KEEP_ALIVE;
      break;
    case Hash::calculateHash("Upgrade"):
      connection = Connection::UPGRADE;
      break;
    default:
      return ResultCode_t::UNKNOWN_HASH +
             ("Request header Connection: " + header.value.getString());
  }
  return ResultCode_t::SUCCESS;
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
 * @return const RequestHeaders::Connection
 */
const RequestHeaders::Connection RequestHeaders::getConnection() const {
  return connection;
}

} // namespace Web