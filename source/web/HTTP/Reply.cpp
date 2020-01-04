#include "Reply.h"

namespace Ehbanana {
namespace Web {
namespace HTTP {

/**
 * @brief Construct a new Reply:: Reply object
 *
 */
Reply::Reply() {}

/**
 * @brief Destroy the Reply:: Reply object
 *
 */
Reply::~Reply() {}

/**
 * @brief Set the HTTP status of the reply
 *
 * @param httpStatus to set
 */
void Reply::setStatus(Status_t httpStatus) {
  status = httpStatus;
}

/**
 * @brief Add the Connection header based on keepAlive
 *
 * @param keepAlive adds "keep-alive" if true, "close" otherwise
 */
void Reply::setKeepAlive(bool keepAlive) {
  if (keepAlive)
    addHeader("Connection", "keep-alive");
  else
    addHeader("Connection", "close");
}

/**
 * @brief Add a header (name and value) to the queue
 *
 * @param name of the header
 * @param value of the header
 */
void Reply::addHeader(const std::string & name, const std::string & value) {
  headers.push_back({name, value});
}

/**
 * @brief Append a string to the content string
 *
 * @param string to append
 */
void Reply::appendContent(std::string string) {
  content.append(string);
}

/**
 * @brief Set the content to a file
 * file will be closed after it is written or connection closes
 *
 * @param contentFile to set
 */
void Reply::setContent(std::shared_ptr<MemoryMapped> contentFile) {
  file = contentFile;
}

/**
 * @brief Get the next set of buffers ready to send
 *
 * @return std::vector<asio::const_buffer> buffers
 */
const std::vector<asio::const_buffer> & Reply::getBuffers() {
  // If the buffers vector is empty, populate first
  if (buffers.empty()) {
    buffers.push_back(statusToBuffer());
    buffers.push_back(asio::buffer(STRING_CRLF));

    // Add content-length
    if (!content.empty())
      addHeader("Content-Length", std::to_string(content.size()));
    else if (file != nullptr)
      addHeader("Content-Length", std::to_string(file->size()));

    for (const Header_t & header : headers) {
      buffers.push_back(asio::buffer(header.name));
      buffers.push_back(asio::buffer(STRING_NAME_VALUE_SEPARATOR));
      buffers.push_back(asio::buffer(header.value));
      buffers.push_back(asio::buffer(STRING_CRLF));
    }
    buffers.push_back(asio::buffer(STRING_CRLF));
    if (!content.empty())
      buffers.push_back(asio::buffer(content));
    else if (file != nullptr)
      buffers.push_back(
          asio::buffer(file->getData(), static_cast<size_t>(file->size())));
  }
  return buffers;
}

/**
 * @brief Generate a stock reply from the HTTP status
 *
 * @param status
 * @return reply
 */
Reply Reply::stockReply(Status_t httpStatus) {
  Reply reply;
  reply.setStatus(httpStatus);
  switch (httpStatus) {
    case Status_t::OK:
      reply.appendContent(StockReply::OK);
      break;
    case Status_t::CREATED:
      reply.appendContent(StockReply::CREATED);
      break;
    case Status_t::ACCEPTED:
      reply.appendContent(StockReply::ACCEPTED);
      break;
    case Status_t::NO_CONTENT:
      reply.appendContent(StockReply::NO_CONTENT);
      break;
    case Status_t::MULTIPLE_CHOICES:
      reply.appendContent(StockReply::MULTIPLE_CHOICES);
      break;
    case Status_t::MOVED_PERMANENTLY:
      reply.appendContent(StockReply::MOVED_PERMANENTLY);
      break;
    case Status_t::MOVED_TEMPORARILY:
      reply.appendContent(StockReply::MOVED_TEMPORARILY);
      break;
    case Status_t::NOT_MODIFIED:
      reply.appendContent(StockReply::NOT_MODIFIED);
      break;
    case Status_t::BAD_REQUEST:
      reply.appendContent(StockReply::BAD_REQUEST);
      break;
    case Status_t::UNAUTHORIZED:
      reply.appendContent(StockReply::UNAUTHORIZED);
      break;
    case Status_t::FORBIDDEN:
      reply.appendContent(StockReply::FORBIDDEN);
      break;
    case Status_t::NOT_FOUND:
      reply.appendContent(StockReply::NOT_FOUND);
      break;
    case Status_t::INTERNAL_SERVER_ERROR:
    default:
      reply.appendContent(StockReply::INTERNAL_SERVER_ERROR);
      break;
    case Status_t::NOT_IMPLEMENTED:
      reply.appendContent(StockReply::NOT_IMPLEMENTED);
      break;
    case Status_t::BAD_GATEWAY:
      reply.appendContent(StockReply::BAD_GATEWAY);
      break;
    case Status_t::SERVICE_UNAVAILABLE:
      reply.appendContent(StockReply::SERVICE_UNAVAILABLE);
      break;
  }
  reply.addHeader("Content-Type", "text/html");
  return reply;
}

/**
 * @brief Transform a HTTP status into its string as a buffer
 *
 * @return asio::const_buffer output string
 */
asio::const_buffer Reply::statusToBuffer() {
  switch (status) {
    case Status_t::SWITCHING_PROTOCOLS:
      return asio::buffer(StatusString::SWITCHING_PROTOCOLS);
    case Status_t::OK:
      return asio::buffer(StatusString::OK);
    case Status_t::CREATED:
      return asio::buffer(StatusString::CREATED);
    case Status_t::ACCEPTED:
      return asio::buffer(StatusString::ACCEPTED);
    case Status_t::NO_CONTENT:
      return asio::buffer(StatusString::NO_CONTENT);
    case Status_t::MULTIPLE_CHOICES:
      return asio::buffer(StatusString::MULTIPLE_CHOICES);
    case Status_t::MOVED_PERMANENTLY:
      return asio::buffer(StatusString::MOVED_PERMANENTLY);
    case Status_t::MOVED_TEMPORARILY:
      return asio::buffer(StatusString::MOVED_TEMPORARILY);
    case Status_t::NOT_MODIFIED:
      return asio::buffer(StatusString::NOT_MODIFIED);
    case Status_t::BAD_REQUEST:
      return asio::buffer(StatusString::BAD_REQUEST);
    case Status_t::UNAUTHORIZED:
      return asio::buffer(StatusString::UNAUTHORIZED);
    case Status_t::FORBIDDEN:
      return asio::buffer(StatusString::FORBIDDEN);
    case Status_t::NOT_FOUND:
      return asio::buffer(StatusString::NOT_FOUND);
    case Status_t::INTERNAL_SERVER_ERROR:
    default:
      return asio::buffer(StatusString::INTERNAL_SERVER_ERROR);
    case Status_t::NOT_IMPLEMENTED:
      return asio::buffer(StatusString::NOT_IMPLEMENTED);
    case Status_t::BAD_GATEWAY:
      return asio::buffer(StatusString::BAD_GATEWAY);
    case Status_t::SERVICE_UNAVAILABLE:
      return asio::buffer(StatusString::SERVICE_UNAVAILABLE);
  }
}

} // namespace HTTP
} // namespace Web
} // namespace Ehbanana