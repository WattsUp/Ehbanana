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
  if (size() != 0)
    debug("Non-zero stream destroyed");
  if (available() != 0)
    debug("Non-zero available stream destroyed");
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

  if ((cursorWrite & CURSOR_MASK) == 0) {
    mutex.lock();
    // Write cursor is at end of region
    if (writeRegion.use_count() == 1) {
      // Read cursor is at a different region, write this one to the file
      fseek(overflowFile, 0, SEEK_END);
      // debug("Write file cursor: " + std::to_string(ftell(overflowFile)));
      if (fwrite(writeRegion->data, 1, REGION_SIZE, overflowFile) !=
          REGION_SIZE)
        throw std::exception("fwrite did not write the entire region");
    }
    writeRegion             = std::make_shared<Region_t>();
    writeRegion->fileOffset = cursorWrite;
    mutex.unlock();
  }
}

/**
 * @brief Write a block to the stream
 *
 * @param buffer
 * @param length
 */
void Stream::write(const uint8_t * buffer, size_t length) {
  size_t  regionCursor = cursorWrite & CURSOR_MASK;
  size_t  regionLeft   = REGION_SIZE - regionCursor;
  size_t  wrote        = std::min(regionLeft, length);
  errno_t err =
      memcpy_s(writeRegion->data + regionCursor, regionLeft, buffer, wrote);
  if (err)
    throw std::exception("Failed to copy memory into stream");
  length -= wrote;
  cursorWrite += wrote;
  while (length != 0) {
    // Write to more regions, make a new one
    mutex.lock();
    // Write cursor is at end of region
    if (writeRegion.use_count() == 1) {
      // Read cursor is at a different region, write this one to the file
      fseek(overflowFile, 0, SEEK_END);
      // debug("Write file cursor: " + std::to_string(ftell(overflowFile)));
      if (fwrite(writeRegion->data, 1, REGION_SIZE, overflowFile) !=
          REGION_SIZE)
        throw std::exception("fwrite did not write the entire region");
    }
    writeRegion             = std::make_shared<Region_t>();
    writeRegion->fileOffset = cursorWrite;
    mutex.unlock();

    // memcpy buffer into region
    buffer += wrote;
    wrote = std::min(REGION_SIZE, length);
    err   = memcpy_s(writeRegion->data, REGION_SIZE, buffer, wrote);
    if (err)
      throw std::exception("Failed to copy memory into stream");
    length -= wrote;
    cursorWrite += wrote;
  }
  if ((cursorWrite & CURSOR_MASK) == 0) {
    mutex.lock();
    // Write cursor is at end of region
    if (writeRegion.use_count() == 1) {
      // Read cursor is at a different region, write this one to the file
      fseek(overflowFile, 0, SEEK_END);
      // debug("Write file cursor: " + std::to_string(ftell(overflowFile)));
      if (fwrite(writeRegion->data, 1, REGION_SIZE, overflowFile) !=
          REGION_SIZE)
        throw std::exception("fwrite did not write the entire region");
    }
    writeRegion             = std::make_shared<Region_t>();
    writeRegion->fileOffset = cursorWrite;
    mutex.unlock();
  }
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
  if ((cursorRead & CURSOR_MASK) == 0) {
    mutex.lock();
    // Read cursor is at end of region
    if ((readRegion->fileOffset + REGION_SIZE) == writeRegion->fileOffset)
      readRegion = writeRegion; // Write region is the next region
    else {
      // Read region from file
      fseek(overflowFile, cursorFileRead, SEEK_SET); // Go to last read region
      // debug("Read file cursor: " + std::to_string(ftell(overflowFile)));
      size_t read =
          fread_s(readRegion->data, REGION_SIZE, 1, REGION_SIZE, overflowFile);
      if (read != REGION_SIZE) {
        if (feof(overflowFile))
          throw std::exception("overflowFile is eof");
        throw std::exception("fread did not read an entire region");
      }
      readRegion->fileOffset += REGION_SIZE;
      cursorFileRead += REGION_SIZE;
    }
    mutex.unlock();
  }
  return c;
}

} // namespace Ehbanana