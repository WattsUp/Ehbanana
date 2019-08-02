#ifndef _WEB_REQUEST_HEADERS_H_
#define _WEB_REQUEST_HEADERS_H_

#include <FruitBowl.h>
#include <asio.hpp>

#include <stdint.h>
#include <string>

namespace Web {

struct HeaderHash_t {
  Hash name;
  Hash value;
};

class RequestHeaders {
public:
  RequestHeaders();

  Result addHeader(HeaderHash_t header);

  enum class Connection : uint8_t { CLOSE, KEEP_ALIVE, UPGRADE };

  const size_t     getContentLength() const;
  const Connection getConnection() const;

private:
  Result addConnection(HeaderHash_t header);

  size_t     contentLength = 0;
  Connection connection    = Connection::CLOSE;
};

} // namespace Web

#endif /* _WEB_REQUEST_HEADERS_H_ */