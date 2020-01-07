#ifndef _EHBANANA_BUFFER_H_
#define _EHBANANA_BUFFER_H_

#include "Ehbanana.h"

#include <asio.hpp>
#include <mutex>
#include <string>
#include <vector>

namespace Ehbanana {

class Buffer {
public:
  Buffer(const Buffer &) = delete;
  Buffer & operator=(const Buffer &) = delete;

  Buffer();
  ~Buffer();

  EBBuffer_t * getHandle();

  size_t available() const;
  void   setComplete(const bool complete);

  bool    write(uint8_t c);
  uint8_t read();

private:
  const static size_t       SIZE        = 0x4000; // Power of 2
  const static size_t       CURSOR_MASK = SIZE - 1;
  std::array<uint8_t, SIZE> data;

  EBBuffer_t handle;
  size_t     cursorRead  = 0;
  size_t     cursorWrite = 0;
};

} // namespace Ehbanana

#endif /* _EHBANANA_BUFFER_H_ */