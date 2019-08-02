#ifndef _WEB_CACHE_CONTROL_H_
#define _WEB_CACHE_CONTROL_H_

#include <FruitBowl.h>
#include <MemoryMapped.h>

#include <list>
#include <regex>
#include <stdint.h>
#include <string>

namespace Web {

struct CacheFilesMatch_t {
  std::regex  regex;
  std::string cacheControlHeader;
};

class CacheControl {
public:
  CacheControl(const CacheControl &) = delete;
  CacheControl & operator=(const CacheControl &) = delete;

  CacheControl(const std::string & fileName);

  std::string getCacheControl(const std::string & fileName);

private:
  Result populateList(const std::string & fileName);
  Result parseTag(const unsigned char *& data, size_t & fileSize,
      CacheFilesMatch_t & filesMatch);

  const std::string DEFAULT = "no-store";

  std::list<CacheFilesMatch_t> cacheFilesMatches;
};

} // namespace Web

#endif /* _WEB_CACHE_CONTROL_H_ */