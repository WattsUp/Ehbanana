#include "Hash.h"
#include <iomanip>
#include <sstream>

/**
 * @brief Get the hash of the next string until the termination characters
 * start: index points to first character to hash
 * end:   index points to the end character (didn't hash)
 *
 * @param file to read
 * @param index to current character
 * @param end1 termination character
 * @param end2 termination character
 * @param end3 termination character
 * @return HashSet_t hash set
 */
HashSet_t Hash::getNextHash(const MemoryMapped & file, uint64_t & index,
    const char end1, const char end2, const char end3) {
  Hash_t      hash   = 0xFFFFFFFF;
  char        buf    = file.at(index);
  std::string string = "";
  while (buf != end1 && buf != end2 && buf != end3) {
    hash = calculateHash(hash, buf);
    string += buf;
    index++;
    buf = file.at(index);
  }
  return {finishHash(hash), string};
}

/**
 * @brief Get the hash of the next string until the termination characters
 * start: index points to first character to hash
 * end:   index points to the end character (didn't hash)
 *
 * @param file to read
 * @param index to current character
 * @param end termination character
 * @return HashSet_t hash set
 */
HashSet_t Hash::getNextHash(
    const MemoryMapped & file, uint64_t & index, const char end) {
  Hash_t      hash   = 0xFFFFFFFF;
  char        buf    = file.at(index);
  std::string string = "";
  while (buf != end) {
    hash = calculateHash(hash, buf);
    string += buf;
    index++;
    buf = file.at(index);
  }
  return {finishHash(hash), string};
}

/**
 * @brief Print out a message about an unknown type with its hash
 *
 * @param file to read
 * @param index to end of hashed string
 * @param type of string hashed
 * @param hash
 * @param stringLength of hashed string
 * @return Results::Result_t UNKNOWN_HASH
 */
Results::Result_t Hash::unknownHash(
    const char * type, const HashSet_t & hashSet) {
  std::ostringstream string;
  string << std::hex << std::setfill('0');
  string << "Unknown hashed " << type << ": 0x" << std::setw(8) << hashSet.hash
         << "=\"" << hashSet.string << "\"";
  return Results::UNKNOWN_HASH + string.str();
}