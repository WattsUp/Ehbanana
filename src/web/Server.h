#ifndef _WEB_SERVER_H_
#define _WEB_SERVER_H_

#include "Result.h"

#include <windows.h>
#include <winsock2.h>
#include <ws2tcpip.h>

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string>

// Need to link with Ws2_32.lib
#pragma comment(lib, "Ws2_32.lib")

namespace Web {

static const char * DEFAULT_PORT_HTTP = "8080";

class Server {
public:
  Server();
  ~Server();

  Results::Result_t initialize(
      const std::string & root, const std::string & port = DEFAULT_PORT_HTTP);
  Results::Result_t run();
  Results::Result_t stop();

private:
  std::string root;
  std::string port;

  SOCKET listenSocket = INVALID_SOCKET;
};

} // namespace Web

#endif /* _WEB_SERVER_H_ */