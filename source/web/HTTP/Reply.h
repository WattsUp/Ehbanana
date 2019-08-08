#ifndef _WEB_REPLY_H_
#define _WEB_REPLY_H_

#include <FruitBowl.h>
#include <MemoryMapped.h>
#include <asio.hpp>

#include <stdint.h>
#include <string>
#include <vector>

namespace Web {
namespace HTTP {

enum class Status_t : uint16_t {
  SWITCHING_PROTOCOLS   = 101,
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
};

namespace StatusString {
// clang-format off
const std::string SWITCHING_PROTOCOLS   = "HTTP/1.0 101 Switching Protocols";
const std::string OK                    = "HTTP/1.0 200 OK";
const std::string CREATED               = "HTTP/1.0 201 Created";
const std::string ACCEPTED              = "HTTP/1.0 202 Accepted";
const std::string NO_CONTENT            = "HTTP/1.0 204 No Content";
const std::string MULTIPLE_CHOICES      = "HTTP/1.0 300 Multiple Choices";
const std::string MOVED_PERMANENTLY     = "HTTP/1.0 301 Moved Permanently";
const std::string MOVED_TEMPORARILY     = "HTTP/1.0 302 Moved Temporarily";
const std::string NOT_MODIFIED          = "HTTP/1.0 304 Not Modified";
const std::string BAD_REQUEST           = "HTTP/1.0 400 Bad Request";
const std::string UNAUTHORIZED          = "HTTP/1.0 401 Unauthorized";
const std::string FORBIDDEN             = "HTTP/1.0 403 Forbidden";
const std::string NOT_FOUND             = "HTTP/1.0 404 Not Found";
const std::string INTERNAL_SERVER_ERROR = "HTTP/1.0 500 Internal Server Error";
const std::string NOT_IMPLEMENTED       = "HTTP/1.0 501 Not Implemented";
const std::string BAD_GATEWAY           = "HTTP/1.0 502 Bad Gateway";
const std::string SERVICE_UNAVAILABLE   = "HTTP/1.0 503 Service Unavailable";
// clang-format on
} // namespace StatusString

namespace StockReply {
// clang-format off
const std::string OK =
    "<html>"
    "<head><title>OK</title></head>"
    "<body><h1>200 OK</h1></body>"
    "</html>";
const std::string CREATED =
    "<html>"
    "<head><title>Created</title></head>"
    "<body><h1>201 Created</h1></body>"
    "</html>";
const std::string ACCEPTED =
    "<html>"
    "<head><title>Accepted</title></head>"
    "<body><h1>202 Accepted</h1></body>"
    "</html>";
const std::string NO_CONTENT =
    "<html>"
    "<head><title>No Content</title></head>"
    "<body><h1>204 No Content</h1></body>"
    "</html>";
const std::string MULTIPLE_CHOICES =
    "<html>"
    "<head><title>Multiple Choices</title></head>"
    "<body><h1>300 Multiple Choices</h1></body>"
    "</html>";
const std::string MOVED_PERMANENTLY =
    "<html>"
    "<head><title>Moved Permanently</title></head>"
    "<body><h1>301 Moved Permanently</h1></body>"
    "</html>";
const std::string MOVED_TEMPORARILY =
    "<html>"
    "<head><title>Moved Temporarily</title></head>"
    "<body><h1>302 Moved Temporarily</h1></body>"
    "</html>";
const std::string NOT_MODIFIED =
    "<html>"
    "<head><title>Not Modified</title></head>"
    "<body><h1>304 Not Modified</h1></body>"
    "</html>";
const std::string BAD_REQUEST =
    "<html>"
    "<head><title>Bad Request</title></head>"
    "<body><h1>400 Bad Request</h1></body>"
    "</html>";
const std::string UNAUTHORIZED =
    "<html>"
    "<head><title>Unauthorized</title></head>"
    "<body><h1>401 Unauthorized</h1></body>"
    "</html>";
const std::string FORBIDDEN =
    "<html>"
    "<head><title>Forbidden</title></head>"
    "<body><h1>403 Forbidden</h1></body>"
    "</html>";
const std::string NOT_FOUND =
    "<html>"
    "<head><title>Not Found</title></head>"
    "<body><h1>404 Not Found</h1></body>"
    "</html>";
const std::string INTERNAL_SERVER_ERROR =
    "<html>"
    "<head><title>Internal Server Error</title></head>"
    "<body><h1>500 Internal Server Error</h1></body>"
    "</html>";
const std::string NOT_IMPLEMENTED =
    "<html>"
    "<head><title> Not Implemented</title></head>"
    "<body><h1>501 Not Implemented</h1></body>"
    "</html>";
const std::string BAD_GATEWAY =
    "<html>"
    "<head><title>Bad Gateway</title></head>"
    "<body><h1>502 Bad Gateway</h1></body>"
    "</html>";
const std::string SERVICE_UNAVAILABLE =
    "<html>"
    "<head><title>Service Unavailable</title></head>"
    "<body><h1>503 Service Unavailable</h1></body>"
    "</html>";
// clang-format on
} // namespace StockReply

const std::string STRING_CRLF                 = "\r\n";
const std::string STRING_NAME_VALUE_SEPARATOR = ": ";

struct Header_t {
  std::string name;
  std::string value;
};

class Reply {
public:
  Reply();
  ~Reply();

  void setStatus(Status_t httpStatus);
  void setKeepAlive(bool keepAlive);
  void addHeader(const std::string & name, const std::string & value);
  void appendContent(std::string string);
  void setContent(MemoryMapped * contentFile);

  const std::vector<asio::const_buffer> & getBuffers();

  bool updateBuffers(size_t bytesWritten = 0);

  static Reply stockReply(Status_t httpStatus);
  static Reply stockReply(Result result);

private:
  asio::const_buffer statusToBuffer();

  MemoryMapped *                  file;
  std::string                     content;
  std::vector<asio::const_buffer> buffers;
  std::vector<Header_t>           headers;
  size_t                          bytesRemaining = 0;
  Status_t                        status = Status_t::OK;
};

} // namespace HTTP
} // namespace Web

#endif /* _WEB_REPLY_H_ */