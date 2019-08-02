#ifndef _WEB_REQUEST_HANDLER_H_
#define _WEB_REQUEST_HANDLER_H_

#include "MIMETypes.h"
#include "Reply.h"
#include "Request.h"

#include <FruitBowl.h>
#include <asio.hpp>

#include <stdint.h>
#include <string>

namespace Web {

class RequestHandler {
public:
  RequestHandler(const RequestHandler &) = delete;
  RequestHandler & operator=(const RequestHandler &) = delete;

  RequestHandler(const std::string & httpRoot, const std::string & configRoot);

  Result handle(const Request & request, Reply & reply);

  void setGUIPort(uint16_t port);

private:
  Result handleGET(const Request & request, Reply & reply);
  Result handlePOST(const Request & request, Reply & reply);
  Result handleEBFile(const Request & request, Reply & reply);

  std::string fileToType(const std::string & file);

  std::string root;
  MIMETypes   mimeTypes;

  uint16_t guiPort;
};

} // namespace Web

#endif /* _WEB_REQUEST_HANDLER_H_ */