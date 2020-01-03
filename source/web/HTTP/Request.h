#ifndef _WEB_REQUEST_H_
#define _WEB_REQUEST_H_

#include "Hash.h"
#include "RequestHeaders.h"

#include <list>
#include <stdint.h>
#include <string>

namespace Ehbanana {
namespace Web {
namespace HTTP {

class Request {
public:
  Request();
  ~Request();

  bool parse(const uint8_t * begin, const uint8_t * end);

  struct Query_t {
    std::string name;
    std::string value;
  };

  HashValue_t                getMethodHash() const;
  const std::string &        getURI() const;
  const std::list<Query_t> & getQueries() const;
  const RequestHeaders &     getHeaders() const;

  bool isParsing();

private:
  void parse(uint8_t c);

  void validateMethod();
  void validateHTTPVersion();

  std::string decodeURI(const std::string & uri);
  uint32_t    decodeHex(const std::string & hex);

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
  Hash httpVersion;

  std::string uri;

  HeaderHash_t currentHeader;

  std::string body;
  std::string endpoint;

  Query_t            currentQuery;
  std::list<Query_t> queries;

  RequestHeaders headers;
};

} // namespace HTTP
} // namespace Web
} // namespace Ehbanana

#endif /* _WEB_REQUEST_H_ */