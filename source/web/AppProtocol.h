#ifndef _WEB_APP_PROTOCOL_H_
#define _WEB_APP_PROTOCOL_H_

#include <FruitBowl.h>
#include <asio.hpp>

namespace Web {

enum class AppProtocol_t : uint8_t { NONE, HTTP, WEBSOCKET };

class AppProtocol {
public:
  AppProtocol(const AppProtocol &) = delete;
  AppProtocol & operator=(const AppProtocol &) = delete;

  /**
   * @brief Construct a new App Protocol object
   *
   */
  AppProtocol() {}

  /**
   * @brief Destroy the App Protocol object
   *
   */
  ~AppProtocol() {}

  /**
   * @brief Process a received buffer, could be the entire message or a fragment
   *
   * @param begin character
   * @param length of buffer
   * @return Result error code
   */
  virtual Result processReceiveBuffer(const char * begin, size_t length) = 0;

  /**
   * @brief Check the completion of the protocol
   *
   * @return true if a message is being processed or waiting for a message
   * @return false if all messages have been processed and no more are expected
   */
  virtual bool isDone() {
    return !hasTransmitBuffers();
  }

  /**
   * @brief Check if there are buffers in the transmit queue that have not been
   * transmitted
   *
   * @return true when the transmit buffers are not empty
   * @return false when the transmit buffers are empty
   */
  bool hasTransmitBuffers() {
    return !buffersTransmit.empty();
  }

  /**
   * @brief Get the Transmit Buffers
   *
   * @return const std::vector<asio::const_buffer>&
   */
  const std::vector<asio::const_buffer> & getTransmitBuffers() {
    return buffersTransmit;
  }

  /**
   * @brief Update the transmit buffers with number of bytes transmitted
   * Removes buffers that have been completely transmitted. Moves the start
   * pointer of the next buffer that has not been transmitted.
   *
   * @param bytesWritten
   * @return true when all transmit buffers have been transmitted
   * @return false when there are more transmit buffers
   */
  virtual bool updateTransmitBuffers(size_t bytesWritten) {
    std::vector<asio::const_buffer>::iterator i   = buffersTransmit.begin();
    std::vector<asio::const_buffer>::iterator end = buffersTransmit.end();
    while (i != end && bytesWritten > 0) {
      asio::const_buffer & buffer = *i;
      if (bytesWritten >= buffer.size()) {
        // This buffer has been written, remove from the queue and decrement
        // bytesWritten
        bytesWritten -= buffer.size();
        i = buffersTransmit.erase(i);
      } else {
        // This buffer has not been fully written, increment its pointer
        buffer += bytesWritten;
        return false;
      }
    }
    return true;
  }

  /**
   * @brief Get the requested protocol to change to
   *
   * @return AppProtocol_t protocol requested, NONE for no change requested
   */
  virtual AppProtocol_t getChangeRequest() {
    return AppProtocol_t::NONE;
  }

protected:
  /**
   * @brief Add a buffer to the transmit queue
   *
   * @param buffer to add
   */
  void addTransmitBuffer(asio::const_buffer buffer) {
    buffersTransmit.push_back(buffer);
  }

  /**
   * @brief Add a buffer to the transmit queue
   *
   * @param buffer to add
   */
  void addTransmitBuffer(const std::vector<asio::const_buffer> & buffers) {
    for (asio::const_buffer buffer : buffers)
      buffersTransmit.push_back(buffer);
  }

private:
  std::vector<asio::const_buffer> buffersTransmit;
};

} // namespace Web

#endif /* _WEB_APP_PROTOCOL_H_ */