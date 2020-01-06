#ifndef _WEB_WEBSOCKET_WEBSOCKET_H_
#define _WEB_WEBSOCKET_WEBSOCKET_H_

#include "..\AppProtocol.h"
#include "Ehbanana.h"
#include "Frame.h"

#include <list>
#include <memory>
#include <string>

namespace Ehbanana {
namespace Web {
namespace WebSocket {

class WebSocket : public AppProtocol {
public:
  WebSocket(const WebSocket &) = delete;
  WebSocket & operator=(const WebSocket &) = delete;

  WebSocket(Server * server);
  ~WebSocket();

  void processReceiveBuffer(const uint8_t * begin, size_t length);

  bool updateTransmitBuffers(size_t bytesWritten);
  bool hasTransmitBuffers();
  bool isDone();
  bool sendAliveCheck();
  void addMessage(const std::string & msg);

private:
  void processFrameText();

  Frame frameIn;

  std::shared_ptr<Frame>             currentFrameOut;
  std::list<std::shared_ptr<Frame> > framesOut;

  bool pingSent = false;
};

} // namespace WebSocket
} // namespace Web
} // namespace Ehbanana

#endif /* _WEB_WEBSOCKET_WEBSOCKET_H_ */