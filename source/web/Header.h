#ifndef _WEB_HEADER_H_
#define _WEB_HEADER_H_

#include <FruitBowl.h>
#include <string>

namespace Web {

struct Header_t {
  std::string name;
  std::string value;
};

struct HeaderHash_t {
  Hash name;
  Hash value;
};

} // namespace Web

#endif /* _WEB_HEADER_H_ */