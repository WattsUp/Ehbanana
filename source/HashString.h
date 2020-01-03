#ifndef _EHBANANA_HashString_STRING_H_
#define _EHBANANA_HashString_STRING_H_

#include "Hash.h"

namespace Ehbanana {

class HashString : public Hash {
private:
  std::string string;

public:
  /**
   * @brief Add a character to the HashString
   *
   * @param c to add
   */
  inline void add(const char c) {
    Hash::add(c);
    string += c;
  }

  const std::string & getString() const {
    return string;
  }
};
} // namespace Ehbanana

#endif /* _EHBANANA_HashString_STRING_H_ */