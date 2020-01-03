#include "MIMETypes.h"

#include "EhbananaLog.h"
#include "Utils.h"

namespace Ehbanana {
namespace Web {
namespace HTTP {

/**
 * @brief Populates the list of types from a file
 * File structure:
 * Each line contains one type: .htm text/html
 *
 * @param filename to parse
 * @throw std::exception Thrown on failure
 */
void MIMETypes::populateList(const std::string & filename) {
  MemoryMapped file(filename, 0, MemoryMapped::SequentialScan);
  if (!file.isValid())
    throw std::exception(("Failed to open MIME types: " + filename).c_str());
  info("Loading MIME types from \"" + filename + "\"");

  size_t                fileSize = static_cast<size_t>(file.size());
  const unsigned char * data     = file.getData();

  while (fileSize > 0) {
    if (*data == '\r') {
      ++data;
      --fileSize;
    }
    if (*data == '\n') {
      ++data;
      --fileSize;
    }

    std::string extension;
    while (*data != ' ' && fileSize != 0) {
      extension += *data;
      ++data;
      --fileSize;
    }
    ++data; // Skip over the space
    --fileSize;

    std::string mimeType;
    while (*data != '\r' && fileSize != 0) {
      mimeType += *data;
      ++data;
      --fileSize;
    }

    types.emplace(extension, mimeType);
  }
  file.close();
}

/**
 * @brief Get the MIME type of the file name
 *
 * @param filename to parse
 * @return const std::string& MIME type
 */
const std::string & MIMETypes::getType(const std::string & filename) const {
  // Extract the file extentsion
  std::string ext = fileExtension(filename);

  try {
    return types.at(ext);
  } catch (const std::exception &) {
    warn("Could not find MIME type for \"" + ext + "\"");
    return UNKNOWN_MIME_TYPE;
  }
}

} // namespace HTTP
} // namespace Web
} // namespace Ehbanana