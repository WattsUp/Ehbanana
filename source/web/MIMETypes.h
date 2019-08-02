#ifndef _WEB_MIME_TYPES_H_
#define _WEB_MIME_TYPES_H_

#include <FruitBowl.h>
#include <MemoryMapped.h>

#include <list>
#include <stdint.h>
#include <string>

namespace Web {
struct MIMEType_t {
  const HashValue_t fileExtension;
  const std::string type;
  uint32_t          usage = 0;
};

bool operator>(const MIMEType_t & left, const MIMEType_t & right);

class MIMETypes {
public:
  MIMETypes(const MIMETypes &) = delete;
  MIMETypes & operator=(const MIMETypes &) = delete;

  MIMETypes(const std::string & fileName);

  const std::string & getType(const std::string & extension);

private:
  Result populateList(const std::string & fileName);

  void sortList();

  const uint32_t    SORT_TIMER_RESET  = 100;
  const std::string UNKNOWN_MIME_TYPE = "application/octet-stream";

  uint32_t sortTimer = SORT_TIMER_RESET;

  std::list<MIMEType_t> typeBuckets[16];
};

} // namespace Web

#endif /* _WEB_MIME_TYPES_H_ */