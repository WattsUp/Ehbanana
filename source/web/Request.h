#ifndef _WEB_REQUEST_H_
#define _WEB_REQUEST_H_

#include "Hash.h"
#include "Header.h"
#include "ResultMsg.h"

#include <asio.hpp>

#include <stdint.h>
#include <string>
#include <vector>

namespace Web {

class Request {
public:
  Request(const Request &) = delete;
  Request & operator=(const Request &) = delete;

  Request(asio::ip::tcp::endpoint endpoint);

  void reset();

  EBResultMsg_t parse(char * begin, char * end);

  const HashSet_t &                 getMethod() const;
  const HashSet_t &                 getURI() const;
  const std::vector<HeaderHash_t> & getQueries() const;
  std::string                       getEndpointString() const;

  bool isKeepAlive();
  bool isParsing();

private:
  EBResultMsg_t parse(char c);

  EBResultMsg_t validateMethod();
  EBResultMsg_t validateHTTPVersion();

  EBResultMsg_t decodeURI(std::string & uri);
  EBResultMsg_t decodeHex(const std::string & hex, uint32_t & value);

  enum class ParsingState_t : uint8_t {
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

  ParsingState_t state = ParsingState_t::IDLE;

  HashSet_t method;
  HashSet_t uri;
  HashSet_t httpVersion;

  size_t contentLength = 0;
  bool   keepAlive     = false;

  std::string             body;
  asio::ip::tcp::endpoint endpoint;

  std::vector<HeaderHash_t> queries;
  std::vector<HeaderHash_t> headers;
};

} // namespace Web

#endif /* _WEB_REQUEST_H_ */