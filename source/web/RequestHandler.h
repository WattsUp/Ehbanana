#ifndef _WEB_REQUEST_HANDLER_H_
#define _WEB_REQUEST_HANDLER_H_

#include "MIMETypes.h"
#include "Reply.h"
#include "Request.h"
#include "Result.h"

#include <asio.hpp>

#include <stdint.h>
#include <string>

namespace Web {

class RequestHandler {
public:
  RequestHandler(const RequestHandler &) = delete;
  RequestHandler & operator=(const RequestHandler &) = delete;

  RequestHandler(const std::string & httpRoot, const std::string & configRoot);

  EBResult_t handle(const Request & request, Reply & reply);

private:
  EBResult_t handleGET(const Request & request, Reply & reply);
  EBResult_t handlePOST(const Request & request, Reply & reply);

  std::string fileToType(const std::string & file);

  std::string root;
  MIMETypes   mimeTypes;
};

} // namespace Web

#endif /* _WEB_REQUEST_HANDLER_H_ */