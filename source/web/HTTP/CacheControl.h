#ifndef _WEB_CACHE_CONTROL_H_
#define _WEB_CACHE_CONTROL_H_

#include "Ehbanana.h"

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

  void populateList(const std::string & filename);

  std::string getCacheControl(const std::string & uri);

private:
  /**
   * @brief Construct a new Cache Control object
   *
   */
  CacheControl() {}

  void parseTag(
      const uint8_t *& data, size_t & fileSize, CacheFilesMatch_t & filesMatch);

  const std::string DEFAULT = "no-store";

  std::list<CacheFilesMatch_t> cacheFilesMatches;
};

} // namespace HTTP
} // namespace Web
} // namespace Ehbanana

#endif /* _WEB_CACHE_CONTROL_H_ */