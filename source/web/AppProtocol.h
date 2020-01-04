#ifndef _WEB_APP_PROTOCOL_H_
#define _WEB_APP_PROTOCOL_H_

#include "Ehbanana.h"

#include <asio.hpp>

namespace Ehbanana {
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
  virtual ~AppProtocol() {}

  /**
   * @brief Process a received buffer, could be the entire message or a fragment
   *
   * @param begin character
   * @param length of buffer
   */
  virtual void processReceiveBuffer(const uint8_t * begin, size_t length) = 0;

  /**
   * @brief Check the completion of the protocol
   *
   * @return true if all messages have been processed and no more are expected
   * @return false if a message is being processed or waiting for a message
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
  virtual bool hasTransmitBuffers() {
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
   */
  void updateTransmitBuffers(size_t bytesWritten) {
    auto it = buffersTransmit.begin();
    while (it != buffersTransmit.end() && bytesWritten > 0) {
      asio::const_buffer & buffer = *it;
      if (bytesWritten >= buffer.size()) {
        // This buffer has been written, remove from the queue and decrement
        // bytesWritten
        bytesWritten -= buffer.size();
        it = buffersTransmit.erase(it);
      } else {
        // This buffer has not been fully written, increment its pointer
        buffer += bytesWritten;
        return;
      }
    }
  }

  /**
   * @brief Get the requested protocol to change to
   *
   * @return AppProtocol_t protocol requested, NONE for no change requested
   */
  virtual AppProtocol_t getChangeRequest() {
    return AppProtocol_t::NONE;
  }

  /**
   * @brief Send a check to test the connection for aliveness
   *
   * @return true when the protocol has already sent an alive check
   * @return false when the protocol has not sent an alive check yet
   */
  virtual bool sendAliveCheck() {
    return true;
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
} // namespace Ehbanana

#endif /* _WEB_APP_PROTOCOL_H_ */