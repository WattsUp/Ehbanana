#ifndef _WEB_MIME_TYPES_H_
#define _WEB_MIME_TYPES_H_

#include "Hash.h"
#include "Result.h"

#include "MemoryMapped.h"

#include <list>
#include <stdint.h>
#include <string>

namespace Web {
typedef struct MIMEType {
  const Hash_t      fileExtension;
  const std::string type;
  uint32_t          usage = 0;
} MIMEType_t;

bool operator>(const MIMEType & left, const MIMEType & right);

class MIMETypes {
public:
  MIMETypes(const MIMETypes &) = delete;
  MIMETypes & operator=(const MIMETypes &) = delete;

  MIMETypes(const std::string & fileName);

  std::string getType(const std::string & extension);

private:
  Results::Result_t populateList(const std::string & fileName);

  void sortList();

  const uint32_t    SORT_TIMER_RESET  = 100;
  const std::string UNKNOWN_MIME_TYPE = "application/octet-stream";

  uint32_t sortTimer = SORT_TIMER_RESET;

  std::list<MIMEType_t> typeBuckets[16];
};

} // namespace Web

#endif /* _WEB_MIME_TYPES_H_ */