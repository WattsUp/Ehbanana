#ifndef _WEB_REPLY_H_
#define _WEB_REPLY_H_

#include "Header.h"
#include "Result.h"

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
// clang-format off
const std::string OK                    = "HTTP/1.0 200 OK\r\n";
const std::string CREATED               = "HTTP/1.0 201 Created\r\n";
const std::string ACCEPTED              = "HTTP/1.0 202 Accepted\r\n";
const std::string NO_CONTENT            = "HTTP/1.0 204 No Content\r\n";
const std::string MULTIPLE_CHOICES      = "HTTP/1.0 300 Multiple Choices\r\n";
const std::string MOVED_PERMANENTLY     = "HTTP/1.0 301 Moved Permanently\r\n";
const std::string MOVED_TEMPORARILY     = "HTTP/1.0 302 Moved Temporarily\r\n";
const std::string NOT_MODIFIED          = "HTTP/1.0 304 Not Modified\r\n";
const std::string BAD_REQUEST           = "HTTP/1.0 400 Bad Request\r\n";
const std::string UNAUTHORIZED          = "HTTP/1.0 401 Unauthorized\r\n";
const std::string FORBIDDEN             = "HTTP/1.0 403 Forbidden\r\n";
const std::string NOT_FOUND             = "HTTP/1.0 404 Not Found\r\n";
const std::string INTERNAL_SERVER_ERROR = "HTTP/1.0 500 Internal Server Error\r\n";
const std::string NOT_IMPLEMENTED       = "HTTP/1.0 501 Not Implemented\r\n";
const std::string BAD_GATEWAY           = "HTTP/1.0 502 Bad Gateway\r\n";
const std::string SERVICE_UNAVAILABLE   = "HTTP/1.0 503 Service Unavailable\r\n";
// clang-format on
} // namespace HTTPStatusString

namespace HTTPStockResponse {
// clang-format off
const char OK[] =
    "<html>"
    "<head><title>OK</title></head>"
    "<body><h1>200 OK</h1></body>"
    "</html>";
const char CREATED[] =
    "<html>"
    "<head><title>Created</title></head>"
    "<body><h1>201 Created</h1></body>"
    "</html>";
const char ACCEPTED[] =
    "<html>"
    "<head><title>Accepted</title></head>"
    "<body><h1>202 Accepted</h1></body>"
    "</html>";
const char NO_CONTENT[] =
    "<html>"
    "<head><title>No Content</title></head>"
    "<body><h1>204 No Content</h1></body>"
    "</html>";
const char MULTIPLE_CHOICES[] =
    "<html>"
    "<head><title>Multiple Choices</title></head>"
    "<body><h1>300 Multiple Choices</h1></body>"
    "</html>";
const char MOVED_PERMANENTLY[] =
    "<html>"
    "<head><title>Moved Permanently</title></head>"
    "<body><h1>301 Moved Permanently</h1></body>"
    "</html>";
const char MOVED_TEMPORARILY[] =
    "<html>"
    "<head><title>Moved Temporarily</title></head>"
    "<body><h1>302 Moved Temporarily</h1></body>"
    "</html>";
const char NOT_MODIFIED[] =
    "<html>"
    "<head><title>Not Modified</title></head>"
    "<body><h1>304 Not Modified</h1></body>"
    "</html>";
const char BAD_REQUEST[] =
    "<html>"
    "<head><title>Bad Request</title></head>"
    "<body><h1>400 Bad Request</h1></body>"
    "</html>";
const char UNAUTHORIZED[] =
    "<html>"
    "<head><title>Unauthorized</title></head>"
    "<body><h1>401 Unauthorized</h1></body>"
    "</html>";
const char FORBIDDEN[] =
    "<html>"
    "<head><title>Forbidden</title></head>"
    "<body><h1>403 Forbidden</h1></body>"
    "</html>";
const char NOT_FOUND[] =
    "<html>"
    "<head><title>Not Found</title></head>"
    "<body><h1>404 Not Found</h1></body>"
    "</html>";
const char INTERNAL_SERVER_ERROR[] =
    "<html>"
    "<head><title>Internal Server Error</title></head>"
    "<body><h1>500 Internal Server Error</h1></body>"
    "</html>";
const char NOT_IMPLEMENTED[] =
    "<html>"
    "<head><title> Not Implemented</title></head>"
    "<body><h1>501 Not Implemented</h1></body>"
    "</html>";
const char BAD_GATEWAY[] =
    "<html>"
    "<head><title>Bad Gateway</title></head>"
    "<body><h1>502 Bad Gateway</h1></body>"
    "</html>";
const char SERVICE_UNAVAILABLE[] =
    "<html>"
    "<head><title>Service Unavailable</title></head>"
    "<body><h1>503 Service Unavailable</h1></body>"
    "</html>";
// clang-format on
} // namespace HTTPStockResponse

const char STRING_CRLF[]                 = {'\r', '\n'};
const char STRING_NAME_VALUE_SEPARATOR[] = {':', ' '};

class Reply {
public:
  Reply(const Reply &) = delete;
  Reply & operator=(const Reply &) = delete;

  Reply();

  void reset();

  void setStatus(HTTPStatus::Status_t status);
  void setKeepAlive(bool keepAlive);
  void addHeader(const std::string & name, const std::string & value);
  void appendContent(std::string content);

  const std::vector<asio::const_buffer> & getBuffers();

  bool updateBuffers(size_t bytesWritten = 0);

  void stockReply(HTTPStatus::Status_t status);
  void stockReply(Results::Result_t result);

private:
  asio::const_buffer statusToBuffer(HTTPStatus::Status_t status);

  std::string                     content;
  std::vector<asio::const_buffer> buffers;
  std::vector<Header_t>           headers;
  size_t                          bytesRemaining = 0;
  HTTPStatus::Status_t            status;
};

} // namespace Web

#endif /* _WEB_REPLY_H_ */