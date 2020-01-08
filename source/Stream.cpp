#include "Stream.h"

#include "EhbananaLog.h"

#include <algorithm>

#include <io.h>

namespace Ehbanana {

/**
 * @brief Construct a new Stream:: Stream object
 *
 */
Stream::Stream() {
  writeRegion = std::make_shared<Region_t>();
  readRegion  = writeRegion;
  errno_t err = tmpfile_s(&overflowFile);
  if (err)
    throw std::exception("tmpfile failed to open");
}

/**
 * @brief Destroy the Stream:: Stream object
 *
 */
Stream::~Stream() {
  fclose(overflowFile);
}

/**
 * @brief Get the size of stream, total bytes written
 *
 * @return size_t
 */
size_t Stream::size() const {
  return cursorWrite;
}

/**
 * @brief Set the end of file flag
 *
 * @param flag true when no more data will be written
 */
void Stream::setEOF(const bool flag) {
  endOfFile = flag;
}

/**
 * @brief Check the end of file flag
 *
 * @return true when no more data will be written
 * @return false otherwise
 */
bool Stream::eof() const {
  return endOfFile;
}

/**
 * @brief Write to the stream
 *
 * @param c
 */
void Stream::write(uint8_t c) {
  writeRegion->data[(cursorWrite & CURSOR_MASK)] = c;
  ++cursorWrite;

  if ((cursorWrite & CURSOR_MASK) == 0)
    pushRegion();
}

/**
 * @brief Write a block to the stream
 *
 * @param buffer
 * @param length
 */
void Stream::write(const uint8_t * buffer, size_t length) {
  size_t  regionCursor = cursorWrite & CURSOR_MASK;
  size_t  wrote        = std::min(REGION_SIZE - regionCursor, length);
  errno_t err =
      memcpy_s(writeRegion->data + regionCursor, wrote, buffer, wrote);
  if (err)
    throw std::exception("Failed to copy memory into stream");
  length -= wrote;
  cursorWrite += wrote;
  while (length != 0) {
    // Write to more regions, make a new one
    pushRegion();

    // memcpy buffer into region
    buffer += wrote;
    wrote = std::min(REGION_SIZE, length);
    err   = memcpy_s(writeRegion->data, REGION_SIZE, buffer, wrote);
    if (err)
      throw std::exception("Failed to copy memory into stream");
    length -= wrote;
    cursorWrite += wrote;
  }
  if ((cursorWrite & CURSOR_MASK) == 0)
    pushRegion();
}

/**
 * @brief Read from the stream
 *
 * @return uint8_t
 */
uint8_t Stream::read() {
  if (available() == 0)
    throw std::underflow_error("Stream underflow");

  uint8_t c = readRegion->data[(cursorRead & CURSOR_MASK)];
  ++cursorRead;
  if ((cursorRead & CURSOR_MASK) == 0)
    popRegion();
  return c;
}

/**
 * @brief Read a block from the stream
 *
 * @param buffer to read into
 * @param length of the buffer
 * @return size_t number of bytes read
 */
size_t Stream::read(uint8_t * buffer, size_t length) {
  size_t bufferIndex  = 0;
  size_t regionCursor = cursorRead & CURSOR_MASK;
  size_t read =
      std::min(available(), std::min(REGION_SIZE - regionCursor, length));
  errno_t err = memcpy_s(buffer, read, readRegion->data + regionCursor, read);
  if (err)
    throw std::exception("Failed to copy memory from stream");
  length -= read;
  cursorRead += read;
  bufferIndex += read;
  while (length != 0 && available() != 0) {
    // Read from more regions, get a new one
    popRegion();

    // memcpy region into buffer
    read = std::min(available(), std::min(REGION_SIZE, length));
    err  = memcpy_s(buffer + bufferIndex, read, readRegion->data, read);
    if (err)
      throw std::exception("Failed to copy memory into stream");
    length -= read;
    cursorRead += read;
    bufferIndex += read;
  }
  if (available() == 0)
    return bufferIndex;
  if ((cursorRead & CURSOR_MASK) == 0)
    popRegion();
  return bufferIndex;
}

/**
 * @brief Write the writeRegion to the end of the overflow file
 *
 */
void Stream::pushRegion() {
  mutex.lock();
  // Write cursor is at end of region
  if (writeRegion.use_count() == 1) {
    // If Read cursor is at a different region, write this one to the file
    fseek(overflowFile, 0, SEEK_END);
    // debug("Write file cursor: " + std::to_string(ftell(overflowFile)));
    if (fwrite(writeRegion->data, 1, REGION_SIZE, overflowFile) !=
        REGION_SIZE) {
      mutex.unlock();
      throw std::exception("fwrite did not write the entire region");
    }
  } else
    // Else readRegion and writeRegion are the same, make a new one
    writeRegion = std::make_shared<Region_t>();
  writeRegion->fileOffset = cursorWrite;
  mutex.unlock();
}

/**
 * @brief Read the readRegion from the overflow file
 *
 */
void Stream::popRegion() {
  mutex.lock();
  if ((readRegion->fileOffset + REGION_SIZE) == writeRegion->fileOffset)
    readRegion = writeRegion; // Write region is the next region
  else {
    // Read region from file
    fseek(overflowFile, cursorFileRead, SEEK_SET); // Go to last read region
    // debug("Read file cursor: " + std::to_string(ftell(overflowFile)));
    if (fread_s(readRegion->data, REGION_SIZE, 1, REGION_SIZE, overflowFile) !=
        REGION_SIZE) {
      mutex.unlock();
      if (feof(overflowFile))
        throw std::exception("overflowFile is eof");
      throw std::exception("fread did not read an entire region");
    }
    readRegion->fileOffset += REGION_SIZE;
    cursorFileRead += REGION_SIZE;
  }
  mutex.unlock();
}

} // namespace Ehbanana