#ifndef _WEB_HEADER_H_
#define _WEB_HEADER_H_

#include <string>
#include "Hash.h"

namespace Web {

typedef struct Header {
  std::string name;
  std::string value;
} Header_t;

typedef struct HeaderHash {
  HashSet_t name;
  HashSet_t value;
} HeaderHash_t;

} // namespace Web

#endif /* _WEB_HEADER_H_ */