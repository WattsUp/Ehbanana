#ifndef _WEB_REQUEST_HANDLER_H_
#define _WEB_REQUEST_HANDLER_H_

#include "Result.h"
#include "Request.h"
#include "Reply.h"
#include "MIMETypes.h"

#include <asio.hpp>

#include <stdint.h>
#include <string>

namespace Web {

class RequestHandler {
public:
  RequestHandler(const RequestHandler &) = delete;
  RequestHandler & operator=(const RequestHandler &) = delete;

  RequestHandler(const std::string & root);

  Results::Result_t handle(const Request & request, Reply & reply);

private:
  Results::Result_t handleGET(const Request & request, Reply & reply);
  Results::Result_t handlePOST(const Request & request, Reply & reply);

  std::string fileToType(const std::string & file);

  std::string root;
  MIMETypes mimeTypes;
};

} // namespace Web

#endif /* _WEB_REQUEST_HANDLER_H_ */