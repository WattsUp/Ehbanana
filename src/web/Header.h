#ifndef _WEB_HEADER_H_
#define _WEB_HEADER_H_

#include <string>

namespace Web {

typedef struct Header {
  std::string name;
  std::string value;
} Header_t;

} // namespace Web

#endif /* _WEB_HEADER_H_ */