#ifndef _WEB_REQUEST_H_
#define _WEB_REQUEST_H_

#include "Header.h"
#include "Result.h"
#include "Hash.h"

#include <asio.hpp>

#include <stdint.h>
#include <string>
#include <vector>

namespace Web {

class Request {
public:
  Request(const Request &) = delete;
  Request & operator=(const Request &) = delete;

  Request();

  void reset();

  Results::Result_t parse(char * begin, char * end);

  bool isKeepAlive();
  bool isParsing();

private:
  Results::Result_t validateMethod();
  Results::Result_t validateHTTPVersion();

  typedef enum ParsingState {
    IDLE,
    METHOD,
    URI,
    HTTP_VERSION,
    HEADER_NAME,
    HEADER_VALUE,
    BODY_NEWLINE,
    BODY
  }ParsingState_t;

  ParsingState_t state = IDLE;

  HashSet_t method;
  HashSet_t uri;
  HashSet_t httpVersion;

  uint64_t contentLength = 0;
  bool keepAlive = false;

  std::string body;

  std::vector<HeaderHash_t> headers;
};

} // namespace Web

#endif /* _WEB_REQUEST_H_ */