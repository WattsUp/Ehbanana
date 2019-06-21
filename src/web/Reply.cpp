#include "Reply.h"

#include "spdlog/spdlog.h"

namespace Web {

/**
 * @brief Construct a new Reply:: Reply object
 *
 */
Reply::Reply() {}

/**
 * @brief Destroy the Reply:: Reply object
 * Delete attached file if present
 *
 */
Reply::~Reply() {
  if (file != nullptr) {
    file->close();
    delete file;
    file = nullptr;
  }
}

/**
 * @brief Reset all fields to their default state
 *
 */
void Reply::reset() {
  content.clear();
  buffers.clear();
  headers.clear();
  bytesRemaining = 0;
  status         = HTTPStatus::OK;
  if (file != nullptr) {
    file->close();
    delete file;
    file = nullptr;
  }
}

/**
 * @brief Set the HTTP status of the reply
 *
 * @param status to set
 */
void Reply::setStatus(HTTPStatus::Status_t status) {
  this->status = status;
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
 * @param content to append
 */
void Reply::appendContent(std::string content) {
  this->content.append(content);
}

/**
 * @brief Set the content to a file
 * file will be closed after it is written or connection closes
 *
 * @param file to set
 */
void Reply::setContent(MemoryMapped * file) {
  this->file = file;
}

/**
 * @brief Get the next set of buffers ready to send
 *
 * @return std::vector<asio::const_buffer> buffers
 */
const std::vector<asio::const_buffer> & Reply::getBuffers() {
  // If the buffers vector is empty, populate first
  if (buffers.empty()) {
    bytesRemaining = 0;
    buffers.clear();
    buffers.push_back(statusToBuffer(status));
    for (int i = 0; i < headers.size(); ++i) {
      Header_t & header = headers[i];
      buffers.push_back(asio::buffer(header.name));
      buffers.push_back(asio::buffer(STRING_NAME_VALUE_SEPARATOR));
      buffers.push_back(asio::buffer(header.value));
      buffers.push_back(asio::buffer(STRING_CRLF));
    }
    buffers.push_back(asio::buffer(STRING_CRLF));
    if (!content.empty())
      buffers.push_back(asio::buffer(content));
    else if (file != nullptr)
      buffers.push_back(asio::buffer(file->getData(), file->size()));
  }
  return buffers;
}

/**
 * @brief Removes the data that has been written, checks if there are remaining
 * bytes to send
 *
 * @param bytesWritten in the previous read
 * @return true if one or more bytes have yet to be written
 * @return false if all bytes have been written
 */
bool Reply::updateBuffers(size_t bytesWritten) {
  std::vector<asio::const_buffer>::iterator i   = buffers.begin();
  std::vector<asio::const_buffer>::iterator end = buffers.end();
  while (i != end && bytesWritten > 0) {
    asio::const_buffer & buffer = *i;
    if (bytesWritten >= buffer.size()) {
      // This buffer has been written, remove from the queue and decrement
      // bytesWritten
      bytesWritten -= buffer.size();
      i = buffers.erase(i);
    } else {
      // This buffer has not been fully written, increment its pointer
      buffer += bytesWritten;
      return true;
    }
  }
  if (buffers.empty() && file != nullptr) {
    file->close();
    delete file;
    file = nullptr;
  }
  return false;
}

/**
 * @brief Generate a stock reply from the HTTP status
 *
 * @param status
 */
void Reply::stockReply(HTTPStatus::Status_t status) {
  reset();
  setStatus(status);
  switch (status) {
    case HTTPStatus::OK:
      appendContent(HTTPStockResponse::OK);
      break;
    case HTTPStatus::CREATED:
      appendContent(HTTPStockResponse::CREATED);
      break;
    case HTTPStatus::ACCEPTED:
      appendContent(HTTPStockResponse::ACCEPTED);
      break;
    case HTTPStatus::NO_CONTENT:
      appendContent(HTTPStockResponse::NO_CONTENT);
      break;
    case HTTPStatus::MULTIPLE_CHOICES:
      appendContent(HTTPStockResponse::MULTIPLE_CHOICES);
      break;
    case HTTPStatus::MOVED_PERMANENTLY:
      appendContent(HTTPStockResponse::MOVED_PERMANENTLY);
      break;
    case HTTPStatus::MOVED_TEMPORARILY:
      appendContent(HTTPStockResponse::MOVED_TEMPORARILY);
      break;
    case HTTPStatus::NOT_MODIFIED:
      appendContent(HTTPStockResponse::NOT_MODIFIED);
      break;
    case HTTPStatus::BAD_REQUEST:
      appendContent(HTTPStockResponse::BAD_REQUEST);
      break;
    case HTTPStatus::UNAUTHORIZED:
      appendContent(HTTPStockResponse::UNAUTHORIZED);
      break;
    case HTTPStatus::FORBIDDEN:
      appendContent(HTTPStockResponse::FORBIDDEN);
      break;
    case HTTPStatus::NOT_FOUND:
      appendContent(HTTPStockResponse::NOT_FOUND);
      break;
    case HTTPStatus::INTERNAL_SERVER_ERROR:
    default:
      appendContent(HTTPStockResponse::INTERNAL_SERVER_ERROR);
      break;
    case HTTPStatus::NOT_IMPLEMENTED:
      appendContent(HTTPStockResponse::NOT_IMPLEMENTED);
      break;
    case HTTPStatus::BAD_GATEWAY:
      appendContent(HTTPStockResponse::BAD_GATEWAY);
      break;
    case HTTPStatus::SERVICE_UNAVAILABLE:
      appendContent(HTTPStockResponse::SERVICE_UNAVAILABLE);
      break;
  }
  addHeader("Content-Length", std::to_string(content.size()));
  addHeader("Content-Type", "text/html");
}

/**
 * @brief Generate a stock reply from the result
 *
 * @param status
 */
void Reply::stockReply(Results::Result_t result) {
  if (result == Results::SUCCESS)
    stockReply(HTTPStatus::OK);
  else if (result == Results::BAD_COMMAND ||
           result == Results::BUFFER_OVERFLOW ||
           result == Results::INVALID_DATA)
    stockReply(HTTPStatus::BAD_REQUEST);
  else if (result == Results::NOT_SUPPORTED ||
           result == Results::VERSION_NOT_SUPPORTED)
    stockReply(HTTPStatus::NOT_IMPLEMENTED);
  else if (result == Results::OPEN_FAILED)
    stockReply(HTTPStatus::NOT_FOUND);
  else
    stockReply(HTTPStatus::INTERNAL_SERVER_ERROR);
}

/**
 * @brief Transform a HTTP status into its string as a buffer
 *
 * @param status to transform
 * @return asio::const_buffer output string
 */
asio::const_buffer Reply::statusToBuffer(HTTPStatus::Status_t status) {
  switch (status) {
    case HTTPStatus::OK:
      return asio::buffer(HTTPStatusString::OK);
    case HTTPStatus::CREATED:
      return asio::buffer(HTTPStatusString::CREATED);
    case HTTPStatus::ACCEPTED:
      return asio::buffer(HTTPStatusString::ACCEPTED);
    case HTTPStatus::NO_CONTENT:
      return asio::buffer(HTTPStatusString::NO_CONTENT);
    case HTTPStatus::MULTIPLE_CHOICES:
      return asio::buffer(HTTPStatusString::MULTIPLE_CHOICES);
    case HTTPStatus::MOVED_PERMANENTLY:
      return asio::buffer(HTTPStatusString::MOVED_PERMANENTLY);
    case HTTPStatus::MOVED_TEMPORARILY:
      return asio::buffer(HTTPStatusString::MOVED_TEMPORARILY);
    case HTTPStatus::NOT_MODIFIED:
      return asio::buffer(HTTPStatusString::NOT_MODIFIED);
    case HTTPStatus::BAD_REQUEST:
      return asio::buffer(HTTPStatusString::BAD_REQUEST);
    case HTTPStatus::UNAUTHORIZED:
      return asio::buffer(HTTPStatusString::UNAUTHORIZED);
    case HTTPStatus::FORBIDDEN:
      return asio::buffer(HTTPStatusString::FORBIDDEN);
    case HTTPStatus::NOT_FOUND:
      return asio::buffer(HTTPStatusString::NOT_FOUND);
    case HTTPStatus::INTERNAL_SERVER_ERROR:
    default:
      return asio::buffer(HTTPStatusString::INTERNAL_SERVER_ERROR);
    case HTTPStatus::NOT_IMPLEMENTED:
      return asio::buffer(HTTPStatusString::NOT_IMPLEMENTED);
    case HTTPStatus::BAD_GATEWAY:
      return asio::buffer(HTTPStatusString::BAD_GATEWAY);
    case HTTPStatus::SERVICE_UNAVAILABLE:
      return asio::buffer(HTTPStatusString::SERVICE_UNAVAILABLE);
  }
}

} // namespace Web