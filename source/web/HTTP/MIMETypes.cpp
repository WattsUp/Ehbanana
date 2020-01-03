#include "MIMETypes.h"

#include "EhbananaLog.h"

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
  size_t                readCount;

  while (fileSize > 0) {
    if (*data == '\r') {
      ++data;
      --fileSize;
    }
    if (*data == '\n') {
      ++data;
      --fileSize;
    }
    Hash extension;
    readCount = extension.add(data, fileSize, ' ');
    data += readCount + 1;
    fileSize -= readCount + 1;
    std::string mimeType;
    while (*data != '\r' && fileSize != 0) {
      mimeType += *data;
      ++data;
      --fileSize;
    }
    typeBuckets[(extension.get() & 0xF)].push_back(
        {extension.get(), mimeType, 0});
  }
  file.close();
  sortList();
}

/**
 * @brief Get the MIME type of the file name
 *
 * @param filename to parse
 * @return const std::string& MIME type
 */
const std::string & MIMETypes::getType(const std::string & filename) {
  // Extract the file extentsion
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

  // Every SORT_TIMER_RESET times this is called, sort the list to improve fetch
  // performance
  sortTimer--;
  if (sortTimer == 0) {
    sortTimer = SORT_TIMER_RESET;
    sortList();
  }

  // Go to the bucket given by the last nibble of the hash, and search that for
  // the extension
  HashValue_t hash = Hash::calculateHash(fileExtension);
  for (MIMEType_t & type : typeBuckets[(hash & 0xF)]) {
    if (type.fileExtension == hash) {
      ++type.usage;
      return type.type;
    }
  }
  warn("Could not find MIME type for \"" + fileExtension + "\"");
  return UNKNOWN_MIME_TYPE;
}

/**
 * @brief Sort each bucket to place the most used types at the top
 *
 */
void MIMETypes::sortList() {
  for (uint8_t i = 0; i < 16; ++i) {
    std::list<MIMEType_t> & typeBucket = typeBuckets[i];
    typeBucket.sort(std::greater<MIMEType_t>());
  }
}

/**
 * @brief Greater than operator for MIMEType_t comparing their usage rate
 *
 * @param left type
 * @param right type
 * @return left.usage > right.usage
 */
bool operator>(const MIMEType_t & left, const MIMEType_t & right) {
  return left.usage > right.usage;
}

} // namespace HTTP
} // namespace Web
} // namespace Ehbanana