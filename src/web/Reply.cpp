#include "Reply.h"

namespace Web {

/**
 * @brief Construct a new Reply:: Reply object
 *
 */
Reply::Reply() {}

/**
 * @brief Set the HTTP status of the reply
 * 
 * @param status to set
 */
void Reply::setStatus(HTTPStatus::Status_t status) {
  this->status = status;
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
 * @brief Transform the reply into a vector of buffers
 * 
 * @return std::vector<asio::const_buffer> buffers
 */
std::vector<asio::const_buffer> Reply::toBuffers() {
  std::vector<asio::const_buffer> buffers;
  buffers.push_back(statusToBuffer(status));
  for (int i = 0; i < headers.size(); ++i) {
    Header_t & header = headers[i];
    buffers.push_back(asio::buffer(header.name));
    buffers.push_back(asio::buffer(STRING_NAME_VALUE_SEPARATOR));
    buffers.push_back(asio::buffer(header.value));
    buffers.push_back(asio::buffer(STRING_CRLF));
  }
  buffers.push_back(asio::buffer(STRING_CRLF));
  buffers.push_back(asio::buffer(content));
  return buffers;
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
    default:
      return asio::buffer(HTTPStatusString::INTERNAL_ERROR);
  }
}

} // namespace Web