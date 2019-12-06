#ifndef _WEB_CACHE_CONTROL_H_
#define _WEB_CACHE_CONTROL_H_

#include "Ehbanana.h"

#include <FruitBowl.h>
#include <MemoryMapped.h>

#include <list>
#include <regex>
#include <stdint.h>
#include <string>

namespace Ehbanana {
namespace Web {
namespace HTTP {

struct CacheFilesMatch_t {
  std::regex  regex;
  std::string cacheControlHeader;
};

// TODO replace with any addition headers regex thingy
class CacheControl {
public:
  CacheControl(const CacheControl &) = delete;
  CacheControl & operator=(const CacheControl &) = delete;

  /**
   * @brief Get the singleton instance
   *
   * @return CacheControl*
   */
  static CacheControl * Instance() {
    static CacheControl instance;
    return &instance;
  }

  Result populateList(const std::string & fileName);

  std::string getCacheControl(const std::string & fileName);

private:
  /**
   * @brief Construct a new Cache Control object
   *
   */
  CacheControl() {}

  Result parseTag(const unsigned char *& data, size_t & fileSize,
      CacheFilesMatch_t & filesMatch);

  const std::string DEFAULT = "no-store";

  std::list<CacheFilesMatch_t> cacheFilesMatches;
};

} // namespace HTTP
} // namespace Web
} // namespace Ehbanana

#endif /* _WEB_CACHE_CONTROL_H_ */