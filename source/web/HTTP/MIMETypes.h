#ifndef _WEB_MIME_TYPES_H_
#define _WEB_MIME_TYPES_H_

#include <MemoryMapped.h>

#include <stdint.h>
#include <string>
#include <unordered_map>

namespace Ehbanana {
namespace Web {
namespace HTTP {

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

  void populateList(const std::string & filename);

  const std::string & getType(const std::string & filename) const;

private:
  /**
   * @brief Construct a new MIMETypes object
   *
   */
  MIMETypes() {}

  const std::string UNKNOWN_MIME_TYPE = "application/octet-stream";

  std::unordered_map<std::string, std::string> types;
};

} // namespace HTTP
} // namespace Web
} // namespace Ehbanana

#endif /* _WEB_MIME_TYPES_H_ */