#ifndef _WEB_REQUEST_HANDLER_H_
#define _WEB_REQUEST_HANDLER_H_

#include "CacheControl.h"
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

private:
  Result handleGET(const Request & request, Reply & reply);
  Result handlePOST(const Request & request, Reply & reply);
  Result handleUpgrade(const Request & request, Reply & reply);

  std::string fileToType(const std::string & file);

  std::string  root;
  MIMETypes    mimeTypes;
  CacheControl cacheControl;
};

} // namespace Web

#endif /* _WEB_REQUEST_HANDLER_H_ */