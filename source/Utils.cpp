#include "Utils.h"

namespace Ehbanana {

/**
 * @brief Get the file extension of a filename
 * "index.html" returns ".html"
 * "../../index.old.html" returns ".html
 * "../../folder/" returns ""
 * "../../folder" returns ""
 *
 * @param filename
 * @return std::string
 */
std::string fileExtension(const std::string & filename) {
  std::string fileExtension;
  bool        dotPresent = false;
  for (char c : filename) {
    if (c == '/') {
      fileExtension.erase();
      dotPresent = false;
    } else if (c == '.') {
      fileExtension = ".";
      dotPresent    = true;
    } else if (dotPresent)
      fileExtension += c;
  }
  return fileExtension;
}

} // namespace Ehbanana