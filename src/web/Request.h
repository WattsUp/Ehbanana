#ifndef _WEB_REQUEST_H_
#define _WEB_REQUEST_H_

#include "Header.h"
#include "Result.h"

#include <asio.hpp>

#include <stdint.h>
#include <string>
#include <vector>

namespace Web {

class Request {
public:
  Request(const Request &) = delete;
  Request & operator=(const Request &) = delete;

  Request();

  Results::Result_t parse(char * begin, char * end);

private:
  std::string method;
  std::string uri;

  int httpVersionMajor;
  int httpVersionMinor;

  std::vector<Header_t> headers;
};

} // namespace Web

#endif /* _WEB_REQUEST_H_ */