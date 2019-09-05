#ifndef _WEB_WEBSOCKET_WEBSOCKET_H_
#define _WEB_WEBSOCKET_WEBSOCKET_H_

#include "..\AppProtocol.h"
#include "Ehbanana.h"
#include "Frame.h"

#include <list>
#include <string>

namespace Ehbanana {
namespace Web {
namespace WebSocket {

class WebSocket : public AppProtocol {
public:
  WebSocket(const WebSocket &) = delete;
  WebSocket & operator=(const WebSocket &) = delete;

  WebSocket(EBGUI_t gui);
  ~WebSocket();

  Result processReceiveBuffer(const uint8_t * begin, size_t length);

  bool   hasTransmitBuffers();
  bool   isDone();
  bool   sendAliveCheck();
  Result addMessage(const std::string & msg);

private:
  Result processFrameText();
  Result processFrameBinary();

  Frame frameIn;

  std::list<Frame *> framesOut;

  EBMessage_t msgAwaitingFile;

  bool pingSent = false;

  EBGUI_t gui;
};

} // namespace WebSocket
} // namespace Web
} // namespace Ehbanana

#endif /* _WEB_WEBSOCKET_WEBSOCKET_H_ */