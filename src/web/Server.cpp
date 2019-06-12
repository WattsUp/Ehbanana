#include "Server.h"
#include "spdlog/spdlog.h"

namespace Web {

Server::Server() {}

Server::~Server() {
  WSACleanup();
}

Results::Result_t Server::initialize(
    const std::string & root, const std::string & port) {
  this->root = root;
  this->port = port;

  WSADATA wsaData;
  int     iResult;

  spdlog::debug("Initializing Winsock");
  iResult = WSAStartup(MAKEWORD(2, 2), &wsaData);
  if (iResult != 0) {
    return Results::OPEN_FAILED + ("WSAStartup failed with error " + iResult);
  }

  struct addrinfo * addr = nullptr;
  struct addrinfo   hints;

  ZeroMemory(&hints, sizeof(hints));
  hints.ai_family   = AF_INET;
  hints.ai_socktype = SOCK_STREAM;
  hints.ai_protocol = IPPROTO_TCP;
  hints.ai_flags    = AI_PASSIVE;

  spdlog::debug("Resolving server address and port");
  iResult = getaddrinfo(NULL, port.c_str(), &hints, &addr);
  if (iResult != 0) {
    return Results::OPEN_FAILED + ("getaddrinfo failed with error " + iResult);
  }

  spdlog::debug("Creating socket to accept new connections");
  listenSocket = socket(addr->ai_family, addr->ai_socktype, addr->ai_protocol);
  if (listenSocket == INVALID_SOCKET) {
    freeaddrinfo(addr);
    return Results::OPEN_FAILED +
           ("socket failed with error " + WSAGetLastError());
  }

  spdlog::debug("Binding socket to address");
  iResult = bind(listenSocket, addr->ai_addr, (int)addr->ai_addrlen);
  if (iResult == SOCKET_ERROR) {
    freeaddrinfo(addr);
    closesocket(listenSocket);
    return Results::OPEN_FAILED +
           ("socket bind failed with error " + WSAGetLastError());
  }

  freeaddrinfo(addr);

  spdlog::debug("Placing socket in listening state");
  iResult = listen(listenSocket, SOMAXCONN);
  if (iResult == SOCKET_ERROR) {
    closesocket(listenSocket);
    return Results::OPEN_FAILED +
           ("socket listen failed with error " + WSAGetLastError());
  }

  return Results::SUCCESS;
}

Results::Result_t Server::run() {
  int iResult = 0;
  while (true) {
    SOCKET client = accept(listenSocket, NULL, NULL);
    char   recvbuf[512];
    int    recvbuflen = 512;
    if (client == INVALID_SOCKET) {
      closesocket(listenSocket);
      return Results::OPEN_FAILED +
             ("socket accept failed with error " + WSAGetLastError());
    }
    // Receive until the peer shuts down the connection
    do {
      iResult = recv(client, recvbuf, recvbuflen, 0);
      if (iResult > 0) {
        spdlog::debug("Bytes received: {}\n{}", iResult, recvbuf);
      } else if (iResult == 0)
        spdlog::debug("Connection closing...");
      else {
        closesocket(client);
        return Results::OPEN_FAILED +
               ("socket recv failed with error " + WSAGetLastError());
      }

    } while (iResult > 0);

    // shutdown the connection since we're done
    iResult = shutdown(client, SD_SEND);
    if (iResult == SOCKET_ERROR) {
      closesocket(client);
      return Results::OPEN_FAILED +
             ("socket shutdown failed with error " + WSAGetLastError());
    }

    // cleanup
    closesocket(client);
  }
  return Results::NOT_SUPPORTED + "Server run";
}

Results::Result_t Server::stop() {
  return Results::NOT_SUPPORTED + "Server stop";
}

} // namespace Web