#ifndef _WEB_REQUEST_HEADERS_H_
#define _WEB_REQUEST_HEADERS_H_

#include "Hash.h"
#include "HashString.h"

#include <asio.hpp>

#include <stdint.h>
#include <string>

namespace Ehbanana {
namespace Web {
namespace HTTP {

struct HeaderHash_t {
  Hash       name;
  HashString value;
};

class RequestHeaders {
public:
  RequestHeaders();

  void addHeader(HeaderHash_t header);

  enum class Connection_t : uint8_t { CLOSE, KEEP_ALIVE, UPGRADE };
  enum class Upgrade_t : uint8_t { NOT_SET, WEB_SOCKET };

  const size_t       getContentLength() const;
  const Connection_t getConnection() const;
  const Upgrade_t    getUpgrade() const;

  const std::string & getWebSocketKey() const;
  const HashValue_t   getWebSocketVersionHash() const;

private:
  void addConnection(HeaderHash_t header);
  void addUpgrade(HeaderHash_t header);

  size_t       contentLength = 0;
  Connection_t connection    = Connection_t::CLOSE;
  Upgrade_t    upgrade       = Upgrade_t::NOT_SET;

  std::string webSocketKey;
  Hash        webSocketVersion;
};

} // namespace HTTP
} // namespace Web
} // namespace Ehbanana

#endif /* _WEB_REQUEST_HEADERS_H_ */