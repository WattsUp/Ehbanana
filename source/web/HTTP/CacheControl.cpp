#include "CacheControl.h"

#include "EhbananaLog.h"
#include "Hash.h"

namespace Ehbanana {
namespace Web {
namespace HTTP {

/**
 * @brief Populates the list of types from a file
 * File structure is xml:
 * <filesMatch "regular expression" cache-control="setting"/>
 *
 * @param filename to parse
 * @throw std::exception Thrown on failure
 */
void CacheControl::populateList(const std::string & filename) {
  MemoryMapped file(filename, 0, MemoryMapped::SequentialScan);
  if (!file.isValid())
    throw std::exception(
        ("Failed to open cache control from: " + filename).c_str());
  info("Loading cache control from \"" + filename + "\"");

  size_t          fileSize = static_cast<size_t>(file.size());
  const uint8_t * data     = static_cast<const uint8_t *>(file.getData());

  while (fileSize > 0) {
    switch (*data) {
      case '\r':
      case '\n':
      case '<':
        ++data;
        --fileSize;
        break;
      case '!': {
        ++data;
        --fileSize;
        // XML comment, throw away until "-->"
        // Shift register to compare the last three characters read
        uint32_t lineEnd = 0;
        while (fileSize > 0 && lineEnd != 0x2D2D3E) {
          lineEnd = ((lineEnd << 8) | *data) & 0xFFFFFF;
          ++data;
          --fileSize;
        }
      } break;
      default: {
        CacheFilesMatch_t filesMatch;
        parseTag(data, fileSize, filesMatch);
        cacheFilesMatches.push_back(filesMatch);
      }
    }
  }
  file.close();
}

/**
 * @brief Parse an XML tag from the data stream and save the properties into a
 * CacheFilesMatch_t
 *
 * @param data stream to read
 * @param fileSize length of the stream remaining
 * @param filesMatch to return properties
 * @throw std::exception Thrown on failure
 */
void CacheControl::parseTag(
    const uint8_t *& data, size_t & fileSize, CacheFilesMatch_t & filesMatch) {
  Hash   tag;
  size_t readCount = tag.add(data, fileSize, '"');
  data += readCount + 1;
  fileSize -= readCount + 1;
  if (tag.get() != Hash::calculateHash("filesMatch "))
    throw std::exception("Unknow line in cache.xml");

  tag       = Hash();
  readCount = tag.add(data, fileSize, '"');
  data += readCount + 1;
  fileSize -= readCount + 1;
  // TODO fix filesMatch.regex = std::regex(tag.getString());

  tag       = Hash();
  readCount = tag.add(data, fileSize, '"');
  data += readCount + 1;
  fileSize -= readCount + 1;
  if (tag.get() != Hash::calculateHash(" cache-control="))
    throw std::exception("Unknow line in cache.xml");

  tag       = Hash();
  readCount = tag.add(data, fileSize, '"');
  data += readCount + 1;
  fileSize -= readCount + 1;
  // TODO fix filesMatch.cacheControlHeader = tag.getString();

  // Throw away until the end of tag marker
  tag       = Hash();
  readCount = tag.add(data, fileSize, '>');
  data += readCount + 1;
  fileSize -= readCount + 1;
}

/**
 * @brief Get the cache control setting of the file extension
 *
 * @param extension to parse
 * @return std::string MIME type
 */
std::string CacheControl::getCacheControl(const std::string & filename) {
  for (CacheFilesMatch_t filesMatch : cacheFilesMatches) {
    if (std::regex_match(filename, filesMatch.regex)) {
      return filesMatch.cacheControlHeader;
    }
  }
  warn("Could not find cache control for \"" + filename + "\"");
  return DEFAULT;
}

} // namespace HTTP
} // namespace Web
} // namespace Ehbanana