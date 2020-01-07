#include "Buffer.h"

#include "EhbananaLog.h"

namespace Ehbanana {

/**
 * @brief Construct a new Buffer:: Buffer object
 *
 */
Buffer::Buffer() {
  handle.complete         = false;
  handle._internal_buffer = this;
}

/**
 * @brief Destroy the Buffer:: Buffer object
 *
 */
Buffer::~Buffer() {}

/**
 * @brief Get the API handle of the buffer, to pass in/out of DLL
 *
 * @return EBBuffer_t*
 */
EBBuffer_t * Buffer::getHandle() {
  return &handle;
}

/**
 * @brief Get the number of available bytes
 *
 * @return size_t
 */
size_t Buffer::available() const {
  return (cursorWrite - cursorRead) & CURSOR_MASK;
}

/**
 * @brief Set the complete flag of the buffer
 *
 * @param complete
 */
void Buffer::setComplete(const bool complete) {
  handle.complete = complete;
}

/**
 * @brief Write to the buffer
 *
 * @param c
 * @return
 */
bool Buffer::write(uint8_t c) {
  // TODO maintain a buffer of 0x10000 and write/read to a file if too much
  if (((cursorWrite + 1 - cursorRead) & CURSOR_MASK) == 0)
    return false;
  data[cursorWrite] = c;
  cursorWrite       = (cursorWrite + 1) & CURSOR_MASK;
  return true;
}

/**
 * @brief Read from the buffer
 *
 * @return uint8_t
 */
uint8_t Buffer::read() {
  if (available() == 0)
    throw std::exception("Buffer underflow");
  uint8_t c  = data[cursorRead];
  cursorRead = (cursorRead + 1) & CURSOR_MASK;
  return c;
}

} // namespace Ehbanana