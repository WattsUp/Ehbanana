#ifndef _WEB_WEBSOCKET_WEBSOCKET_H_
#define _WEB_WEBSOCKET_WEBSOCKET_H_

#include "..\AppProtocol.h"

namespace Web {
namespace WebSocket {

class WebSocket : public AppProtocol {
public:
  WebSocket(const WebSocket &) = delete;
  WebSocket & operator=(const WebSocket &) = delete;

  WebSocket();
  ~WebSocket();

  Result processReceiveBuffer(const char * begin, size_t length);

  bool isDone();
};

} // namespace WebSocket
} // namespace Web

#endif /* _WEB_WEBSOCKET_WEBSOCKET_H_ */