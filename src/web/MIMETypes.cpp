#include "MIMETypes.h"
#include "spdlog/spdlog.h"

namespace Web {

/**
 * @brief Construct a new MIMETypes::MIMETypes object
 *
 * @param fileName to read types from
 */
MIMETypes::MIMETypes(const std::string & fileName) {
  Results::Result_t result = populateList(fileName);
  if (!result) {
    spdlog::error(result);
    throw std::exception(result.message.c_str());
  }
}

/**
 * @brief Populates the list of types from a file
 * File structure:
 * Each line contains one type: .htm text/html
 *
 * @param fileName to parse
 * @return Results::Result_t error code
 */
Results::Result_t MIMETypes::populateList(const std::string & fileName) {
  MemoryMapped file(fileName, 0, MemoryMapped::SequentialScan);
  if (!file.isValid())
    return Results::OPEN_FAILED + fileName;
  spdlog::info("Loading MIME types from \"{}\"", fileName);

  uint64_t index    = 0;
  uint64_t fileSize = file.size();

  while (index < fileSize) {
    if (file.at(index) == '\n')
      ++index;
    HashSet_t extension = Hash::getNextHash(file, index, ' ');
    ++index;
    HashSet_t mimeType = Hash::getNextHash(file, index, '\r');
    ++index;
    typeBuckets[(extension.hash & 0xF)].push_back(
        {extension.hash, mimeType.string, 0});
  }
  file.close();
  sortList();
  return Results::SUCCESS;
}

/**
 * @brief Get the MIME type of the file extension
 *
 * @param extension to parse
 * @return const std::string& MIME type
 */
std::string MIMETypes::getType(const std::string & extension) {
  // Every SORT_TIMER_RESET times this is called, sort the list to improve fetch
  // performance
  sortTimer--;
  if (sortTimer == 0) {
    sortTimer = SORT_TIMER_RESET;
    sortList();
  }

  // Go to the bucket given by the last nibble of the hash, and search that for
  // the extension
  Hash_t hash = Hash::calculateHash(extension);
  for (MIMEType_t & type : typeBuckets[(hash & 0xF)]) {
    if (type.fileExtension == hash) {
      ++type.usage;
      return type.type;
    }
  }
  spdlog::warn("Could not find MIME type for \"{}\"", extension);
  return UNKNOWN_MIME_TYPE;
}

/**
 * @brief Sort each bucket to place the most used types at the top
 *
 */
void MIMETypes::sortList() {
  for (uint8_t i = 0; i < 16; i++) {
    std::list<MIMEType_t> & typeBucket = typeBuckets[i];
    typeBucket.sort(std::greater<MIMEType_t>());
  }
}

/**
 * @brief Greater than operator for MIMEType comparing their usage rate
 *
 * @param left type
 * @param right type
 * @return left.usage > right.usage
 */
bool operator>(const MIMEType & left, const MIMEType & right) {
  return left.usage > right.usage;
}

} // namespace Web