#ifndef _WEB_REQUEST_H_
#define _WEB_REQUEST_H_

#include "RequestHeaders.h"

#include <FruitBowl.h>

#include <stdint.h>
#include <string>
#include <vector>

namespace Web {
namespace HTTP {

class Request {
public:
  Request();
  ~Request();

  Result parse(const char * begin, const char * end);

  const Hash &                      getMethod() const;
  const Hash &                      getURI() const;
  const std::vector<HeaderHash_t> & getQueries() const;
  const RequestHeaders &            getHeaders() const;

  bool isParsing();

private:
  Result parse(char c);

  Result validateMethod();
  Result validateHTTPVersion();

  Result decodeURI(Hash & uriHash);
  Result decodeHex(const std::string & hex, uint32_t & value);

  enum class State_t : uint8_t {
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
  };

  State_t state = State_t::IDLE;

  Hash method;
  Hash uri;
  Hash httpVersion;

  HeaderHash_t currentHeader;

  std::string body;
  std::string endpoint;

  std::vector<HeaderHash_t> queries;

  RequestHeaders headers;
};

} // namespace HTTP
} // namespace Web

#endif /* _WEB_REQUEST_H_ */