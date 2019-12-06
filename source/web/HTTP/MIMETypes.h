#ifndef _WEB_MIME_TYPES_H_
#define _WEB_MIME_TYPES_H_

#include <FruitBowl.h>
#include <MemoryMapped.h>

#include <list>
#include <stdint.h>
#include <string>

namespace Ehbanana {
namespace Web {
namespace HTTP {

struct MIMEType_t {
  const HashValue_t fileExtension;
  const std::string type;
  uint32_t          usage = 0;
};

bool operator>(const MIMEType_t & left, const MIMEType_t & right);
// TODO replace with unordered list hash table
class MIMETypes {
public:
  MIMETypes(const MIMETypes &) = delete;
  MIMETypes & operator=(const MIMETypes &) = delete;

  /**
   * @brief Get the singleton instance
   *
   * @return MIMETypes*
   */
  static MIMETypes * Instance() {
    static MIMETypes instance;
    return &instance;
  }

  Result populateList(const std::string & fileName);

  const std::string & getType(const std::string & extension);

private:
  /**
   * @brief Construct a new MIMETypes object
   *
   */
  MIMETypes() {}

  void sortList();

  const uint32_t    SORT_TIMER_RESET  = 100;
  const std::string UNKNOWN_MIME_TYPE = "application/octet-stream";

  uint32_t sortTimer = SORT_TIMER_RESET;

  std::list<MIMEType_t> typeBuckets[16];
};

} // namespace HTTP
} // namespace Web
} // namespace Ehbanana

#endif /* _WEB_MIME_TYPES_H_ */