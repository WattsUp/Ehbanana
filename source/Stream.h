#ifndef _EHBANANA_STREAM_H_
#define _EHBANANA_STREAM_H_

#include <memory>
#include <mutex>
#include <stdint.h>

namespace Ehbanana {

class Stream {
public:
  Stream(const Stream &) = delete;
  Stream & operator=(const Stream &) = delete;

  Stream();
  ~Stream();

  size_t size() const;

  /**
   * @brief Get the number of available bytes
   *
   * @return size_t
   */
  inline size_t available() const {
    return cursorWrite - cursorRead;
  }

  void setEOF(const bool flag);
  bool eof() const;

  void    write(uint8_t c);
  void    write(const uint8_t * buffer, size_t length);
  uint8_t read();
  size_t  read(uint8_t * buffer, size_t length);

private:
  void pushRegion();
  void popRegion();

  bool endOfFile = false;

  static const size_t REGION_SIZE = 0x10000; // Power of 2
  static const size_t CURSOR_MASK = REGION_SIZE - 1;

  struct Region_t {
    size_t  fileOffset = 0;
    uint8_t data[REGION_SIZE];
  };

  std::shared_ptr<Region_t> writeRegion = nullptr;
  std::shared_ptr<Region_t> readRegion  = nullptr;

  FILE *     overflowFile = nullptr;
  std::mutex mutex;

  size_t cursorRead     = 0;
  size_t cursorFileRead = 0;
  size_t cursorWrite    = 0;
};

} // namespace Ehbanana

#endif /* _EHBANANA_STREAM_H_ */