#ifndef _WEB_REPLY_H_
#define _WEB_REPLY_H_

#include "Header.h"

#include <asio.hpp>

#include <stdint.h>
#include <string>
#include <vector>

namespace Web {

namespace HTTPStatus {
typedef enum Status {
  OK                    = 200,
  CREATED               = 201,
  ACCEPTED              = 202,
  NO_CONTENT            = 204,
  MULTIPLE_CHOICES      = 300,
  MOVED_PERMANENTLY     = 301,
  MOVED_TEMPORARILY     = 302,
  NOT_MODIFIED          = 304,
  BAD_REQUEST           = 400,
  UNAUTHORIZED          = 401,
  FORBIDDEN             = 403,
  NOT_FOUND             = 404,
  INTERNAL_SERVER_ERROR = 500,
  NOT_IMPLEMENTED       = 501,
  BAD_GATEWAY           = 502,
  SERVICE_UNAVAILABLE   = 503
} Status_t;
}

namespace HTTPStatusString {
const std::string OK                = "HTTP/1.0 200 OK\r\n";
const std::string CREATED           = "HTTP/1.0 201 Created\r\n";
const std::string ACCEPTED          = "HTTP/1.0 202 Accepted\r\n";
const std::string NO_CONTENT        = "HTTP/1.0 204 No Content\r\n";
const std::string MULTIPLE_CHOICES  = "HTTP/1.0 300 Multiple Choices\r\n";
const std::string MOVED_PERMANENTLY = "HTTP/1.0 301 Moved Permanently\r\n";
const std::string MOVED_TEMPORARILY = "HTTP/1.0 302 Moved Temporarily\r\n";
const std::string NOT_MODIFIED      = "HTTP/1.0 304 Not Modified\r\n";
const std::string BAD_REQUEST       = "HTTP/1.0 400 Bad Request\r\n";
const std::string UNAUTHORIZED      = "HTTP/1.0 401 Unauthorized\r\n";
const std::string FORBIDDEN         = "HTTP/1.0 403 Forbidden\r\n";
const std::string NOT_FOUND         = "HTTP/1.0 404 Not Found\r\n";
const std::string INTERNAL_ERROR    = "HTTP/1.0 500 Internal Server Error\r\n";
const std::string NOT_IMPLEMENTED   = "HTTP/1.0 501 Not Implemented\r\n";
const std::string BAD_GATEWAY       = "HTTP/1.0 502 Bad Gateway\r\n";
const std::string SERV_UNAVAILABLE  = "HTTP/1.0 503 Service Unavailable\r\n";
} // namespace HTTPStatusString

const char STRING_CRLF[]                 = "\r\n";
const char STRING_NAME_VALUE_SEPARATOR[] = ": ";

class Reply {
public:
  Reply(const Reply &) = delete;
  Reply & operator=(const Reply &) = delete;

  Reply();

  void setStatus(HTTPStatus::Status_t status);
  void addHeader(const std::string & name, const std::string & value);
  void appendContent(std::string content);

  std::vector<asio::const_buffer> toBuffers();
  std::string           content;

private:
  asio::const_buffer statusToBuffer(HTTPStatus::Status_t status);

  std::vector<Header_t> headers;
  HTTPStatus::Status_t  status;
};

} // namespace Web

#endif /* _WEB_REPLY_H_ */