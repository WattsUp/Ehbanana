#ifndef _HASH_H_
#define _HASH_H_

#include "MemoryMapped.h"
#include <stdint.h>
#include <string>

typedef uint32_t Hash_t;

struct HashSet_t {
  Hash_t      hash;
  std::string string;
};

class Hash {
public:
  static HashSet_t getNextHash(const MemoryMapped & file, size_t & index,
      const char end1, const char end2, const char end3);
  static HashSet_t getNextHash(
      const MemoryMapped & file, size_t & index, const char end);

  /**
   * @brief Calculate the hash from a string
   *
   * @param string to hash
   * @return Hash_t hash
   */
  static Hash_t calculateHash(const std::string & string) {
    Hash_t hash = 0xFFFFFFFF;
    for (char c : string) {
      hash = calculateHash(hash, c);
    }
    return finishHash(hash);
  }

  /**
   * @brief Calculate the hash from a string
   *
   * @param string to hash
   * @return constexpr Hash_t hash
   */
  static constexpr Hash_t calculateHash(char * string) {
    Hash_t hash = 0xFFFFFFFF;
    while (*string != '\0') {
      hash = calculateHash(hash, *string);
      ++string;
    }
    return finishHash(hash);
  }

private:
  Hash() = delete;

  /**
   * @brief Calculate the hash through its algorithm on the seed hash and char
   * Jenkin's hash function
   *
   * @param hash to seed
   * @param c char to append
   * @return constexpr Hash_t hash
   */
  static constexpr Hash_t calculateHash(Hash_t hash, const char c) {
    hash = static_cast<Hash_t>(hash + static_cast<uint64_t>(c));
    hash = static_cast<Hash_t>(hash + (static_cast<uint64_t>(hash) << 10));
    hash = static_cast<Hash_t>(hash ^ (static_cast<uint64_t>(hash) >> 6));
    return hash;
  }

  /**
   * @brief Applies final operations to finish creating a hash
   *
   * @param hash to process
   * @return constexpr Hash_t final hash
   */
  static constexpr Hash_t finishHash(Hash_t hash) {
    hash = static_cast<Hash_t>(hash + (static_cast<uint64_t>(hash) << 3));
    hash = static_cast<Hash_t>(hash ^ (static_cast<uint64_t>(hash) >> 11));
    hash = static_cast<Hash_t>(hash + (static_cast<uint64_t>(hash) << 15));
    return hash;
  }
};

#endif /* _HASH_H_ */