#ifndef _WEB_HEADER_H_
#define _WEB_HEADER_H_

#include <string>
#include "Hash.h"

namespace Web {

struct Header_t {
  std::string name;
  std::string value;
};

struct HeaderHash_t {
  HashSet_t name;
  HashSet_t value;
};

} // namespace Web

#endif /* _WEB_HEADER_H_ */