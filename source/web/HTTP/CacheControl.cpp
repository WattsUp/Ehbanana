#include "CacheControl.h"

#include "EhbananaLog.h"

namespace Ehbanana {
namespace Web {
namespace HTTP {

/**
 * @brief Populates the list of types from a file
 * File structure is xml:
 * <filesMatch "regular expression" cache-control="setting"/>
 *
 * @param fileName to parse
 * @return Result error code
 */
Result CacheControl::populateList(const std::string & fileName) {
  MemoryMapped file(fileName, 0, MemoryMapped::SequentialScan);
  if (!file.isValid())
    return ResultCode_t::OPEN_FAILED +
           ("Opening cache control from: " + fileName);
  info("Loading cache control from \"" + fileName + "\"");

  size_t                fileSize = static_cast<size_t>(file.size());
  const unsigned char * data     = file.getData();
  Result                result;

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
        uint32_t lineEnd = 0;
        while (fileSize > 0 && lineEnd != 0x2D2D3E) {
          lineEnd = ((lineEnd << 8) | *data) & 0xFFFFFF;
          ++data;
          --fileSize;
        }
      } break;
      default: {
        CacheFilesMatch_t filesMatch;
        result = parseTag(data, fileSize, filesMatch);
        if (!result)
          return result + "Parsing tag from cache.xml";
        cacheFilesMatches.push_back(filesMatch);
      }
    }
  }
  file.close();
  return ResultCode_t::SUCCESS;
}

/**
 * @brief Parse an XML tag from the data stream and save the properties into a
 * CacheFilesMatch_t
 *
 * @param data stream to read
 * @param fileSize length of the stream remaining
 * @param filesMatch to return properties
 * @return Result error code
 */
Result CacheControl::parseTag(const unsigned char *& data, size_t & fileSize,
    CacheFilesMatch_t & filesMatch) {
  Hash   tag;
  size_t readCount = tag.add(data, fileSize, '"');
  data += readCount + 1;
  fileSize -= readCount + 1;
  if (tag.get() != Hash::calculateHash("filesMatch "))
    return ResultCode_t::UNKNOWN_HASH + ("cache.xml: " + tag.getString());

  tag       = Hash();
  readCount = tag.add(data, fileSize, '"');
  data += readCount + 1;
  fileSize -= readCount + 1;
  filesMatch.regex = std::regex(tag.getString());

  tag       = Hash();
  readCount = tag.add(data, fileSize, '"');
  data += readCount + 1;
  fileSize -= readCount + 1;
  if (tag.get() != Hash::calculateHash(" cache-control="))
    return ResultCode_t::UNKNOWN_HASH + ("cache.xml: " + tag.getString());

  tag       = Hash();
  readCount = tag.add(data, fileSize, '"');
  data += readCount + 1;
  fileSize -= readCount + 1;
  filesMatch.cacheControlHeader = tag.getString();

  // Throw away until the end of tag marker
  tag       = Hash();
  readCount = tag.add(data, fileSize, '>');
  data += readCount + 1;
  fileSize -= readCount + 1;

  return ResultCode_t::SUCCESS;
}

/**
 * @brief Get the cache control setting of the file extension
 *
 * @param extension to parse
 * @return const std::string& MIME type
 */
std::string CacheControl::getCacheControl(const std::string & fileName) {
  for (CacheFilesMatch_t filesMatch : cacheFilesMatches) {
    if (std::regex_match(fileName, filesMatch.regex)) {
      return filesMatch.cacheControlHeader;
    }
  }
  warn("Could not find cache control for \"" + fileName + "\"");
  return DEFAULT;
}

} // namespace HTTP
} // namespace Web
} // namespace Ehbanana