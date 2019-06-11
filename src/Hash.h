#ifndef _HASH_H_
#define _HASH_H_

#include "MemoryMapped.h"
#include "Result.h"
#include <string>

typedef uint32_t Hash_t;
typedef struct HashSet {
  Hash_t      hash;
  std::string string;
} HashSet_t;

class Hash {
public:
  static HashSet_t getNextHash(const MemoryMapped & file, uint64_t & index,
      const char end1, const char end2, const char end3);
  static HashSet_t getNextHash(
      const MemoryMapped & file, uint64_t & index, const char end);

  static Results::Result_t unknownHash(
      const char * type, const HashSet_t & hashSet);

  /**
   * @brief Calculate the hash from a string
   *
   * @param string to hash
   * @return constexpr Hash_t hash
   */
  static constexpr Hash_t calculateHash(char * string) {
    uint32_t hash = 0xFFFFFFFF;
    while (*string != '\0') {
      hash = calculateHash(hash, *string);
      ++string;
    }
    return finishHash(hash);
  }

private:
  /**
   * @brief Calculate the hash through its algorithm on the seed hash and char
   * Jenkin's hash function
   *
   * @param hash to seed
   * @param c char to append
   * @return constexpr Hash_t hash
   */
  static constexpr Hash_t calculateHash(Hash_t hash, const char c) {
    hash += c;
    hash += hash << 10;
    hash ^= hash >> 6;
    return hash;
  }

/**
 * @brief Applies final operations to finish creating a hash
 * 
 * @param hash to process
 * @return constexpr Hash_t final hash
 */
  static constexpr Hash_t finishHash(Hash_t hash) {
    hash += hash << 3;
    hash ^= hash >> 11;
    hash += hash << 15;
    return hash;
  }
};

#endif /* _HASH_H_ */