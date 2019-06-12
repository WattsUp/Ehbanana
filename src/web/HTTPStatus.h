#ifndef _WEB_HTTPSTATUS_H_
#define _WEB_HTTPSTATUS_H_

#include <stdint.h>
#include <string>
#include <iostream>

namespace Web {

namespace HTTPStatus {

typedef struct Status {
  uint16_t    value;
  std::string message;
} Status_t;

static const Status_t CONTINUE            = {100, "Continue"};
static const Status_t SWITCHING_PROTOCOLS = {101, "Switching Protocols"};

static const Status_t OK                    = {200, "OK"};
static const Status_t CREATED               = {201, "Created"};
static const Status_t ACCEPTED              = {202, "Accepted"};
static const Status_t NON_AUTHORIATIVE_INFO = {203, "Non-authoritative Information"};
static const Status_t NO_CONTENT            = {204, "No Content"};
static const Status_t RESET_CONTENT         = {205, "Reset Content"};
static const Status_t PARTIAL_CONTENT       = {206, "Partial Content"};

static const Status_t MULTIPLE_CHOICES   = {300, "Multiple Choices"};
static const Status_t MOVED_PERMANENTLY  = {301, "Moved Permanently"};
static const Status_t FOUND              = {302, "Found"};
static const Status_t SEE_OTHER          = {303, "See Other"};
static const Status_t NOT_MODIFIED       = {304, "Not Modified"};
static const Status_t USE_PROXY          = {305, "Use Proxy"};
static const Status_t TEMPORARY_REDIRECT = {307, "Temporary Redirect"};

static const Status_t BAD_REQUEST                   = {400, "Bad Request"};
static const Status_t UNAUTHORIZED                  = {401, "Unauthorized"};
static const Status_t PAYMENT_REQUIRED              = {402, "Payment Required"};
static const Status_t FORBIDDEN                     = {403, "Forbidden"};
static const Status_t NOT_FOUND                     = {404, "Not Found"};
static const Status_t METHOD_NOT_ALLOWED            = {405, "Method Not Allowed"};
static const Status_t NOT_ACCEPTABLE                = {406, "Not Acceptable"};
static const Status_t PROXY_AUTH_REQUIRED           = {407, "Proxy Authentication Required"};
static const Status_t REQUEST_TIMEOUT               = {408, "Request Timeout"};
static const Status_t CONFLICT                      = {409, "Conflict"};
static const Status_t GONE                          = {410, "Gone"};
static const Status_t LENGTH_REQUIRED               = {411, "Length Required"};
static const Status_t PRECONDITION_FAILED           = {412, "Precondition Failed"};
static const Status_t REQUEST_ENT_TOO_LARGE         = {413, "Request Entity Too Large"};
static const Status_t REQUEST_URL_TOO_LONG          = {414, "Request-url Too Long"};
static const Status_t UNSUPPORTED_MEDIA_TYPE        = {415, "Unsupported Media Type"};
static const Status_t REQUEST_RANGE_NOT_SATISFIABLE = {416, "Requested Range Not Satisfiable"};
static const Status_t EXPECTATION_FAILED            = {417, "Expectation Failed"};

static const Status_t INTERNAL_SERVER_ERROR  = {500, "Internal Server Error"};
static const Status_t NOT_IMPLEMENTED        = {501, "Not Implemented"};
static const Status_t BAD_GATEWAY            = {502, "Bad Gateway"};
static const Status_t SERVICE_UNAVAILABLE    = {503, "Service Unavailable"};
static const Status_t GATEWAY_TIMEOUT        = {504, "Gateway Timeout"};
static const Status_t HTTP_VER_NOT_SUPPORTED = {505, "HTTP Version Not Supported"};

} // namespace HTTPStatus

} // namespace Web

std::ostream & operator<<(
    std::ostream & stream, const Web::HTTPStatus::Status_t & status);

#endif /* _WEB_HTTPSTATUS_H_ */