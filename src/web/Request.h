#ifndef _WEB_REQUEST_H_
#define _WEB_REQUEST_H_

#include "Hash.h"
#include "Header.h"
#include "Result.h"

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

  const HashSet_t &                 getMethod() const;
  const HashSet_t &                 getURI() const;
  const std::vector<HeaderHash_t> & getQueries() const;

  bool isKeepAlive();
  bool isParsing();

private:
  Results::Result_t validateMethod();
  Results::Result_t validateHTTPVersion();

  Results::Result_t decodeURI(std::string & uri);
  Results::Result_t decodeHex(const std::string & hex, uint32_t & value);

  typedef enum ParsingState {
    IDLE,
    METHOD,
    URI,
    QUERY_NAME,
    QUERY_VALUE,
    HTTP_VERSION,
    HEADER_NAME,
    HEADER_VALUE,
    BODY_NEWLINE,
    BODY
  } ParsingState_t;

  ParsingState_t state = IDLE;

  HashSet_t method;
  HashSet_t uri;
  HashSet_t httpVersion;

  uint64_t contentLength = 0;
  bool     keepAlive     = false;

  std::string body;

  std::vector<HeaderHash_t> queries;
  std::vector<HeaderHash_t> headers;
};

} // namespace Web

#endif /* _WEB_REQUEST_H_ */