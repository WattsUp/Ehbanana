#include "Reply.h"

namespace Web {
Reply::Reply() {}

void Reply::setStatus(HTTPStatus::Status_t status) {
  this->status = status;
}

void Reply::addHeader(const std::string & name, const std::string & value) {
  headers.push_back({name, value});
}

void Reply::appendContent(std::string content) {
  this->content.append(content);
}

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

asio::const_buffer Reply::statusToBuffer(HTTPStatus::Status_t status) {
  switch (status) {
    case HTTPStatus::OK:
      return asio::buffer(HTTPStatusString::OK);
    default:
      return asio::buffer(HTTPStatusString::INTERNAL_ERROR);
  }
}

} // namespace Web