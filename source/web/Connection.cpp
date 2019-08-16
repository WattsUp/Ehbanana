#include "Connection.h"

#include <spdlog/spdlog.h>
#include <sstream>

namespace Web {

/**
 * @brief Construct a new Connection:: Connection object
 *
 * @param socket to read from and write to
 * @param endpoint socket is connected to
 * @param now current timestamp
 * @param gui that owns this server
 */
Connection::Connection(asio::ip::tcp::socket * socket, std::string endpoint,
    const std::chrono::time_point<std::chrono::system_clock> & now,
    EBGUI_t                                                    gui) :
  socket(socket),
  endpoint(endpoint), gui(gui) {
  this->timeoutTime = now + TIMEOUT;

  socket->non_blocking(true);

  asio::socket_base::keep_alive option(true);
  socket->set_option(option);
}

/**
 * @brief Destroy the Connection:: Connection object
 * Safely stop the socket
 */
Connection::~Connection() {
  stop();
}

/**
 * @brief Update the current operation based on the protocol
 *
 * Returns ResultCode_t::INCOMPLETE if more operations are required
 * Returns ResultCode_t::NO_OPERATION if nothing happenend this update
 * Returns ResultCode_t::TIMEOUT if the connection was idle for too long
 *
 * @param now current timestamp
 * @return Result error code
 */
Result Connection::update(
    const std::chrono::time_point<std::chrono::system_clock> & now) {
  Result           result;
  asio::error_code errorCode;
  size_t           length = socket->available(errorCode);
  if (errorCode) {
    return ResultCode_t::READ_FAULT + errorCode.message() +
           ("Checking available bytes for " + endpoint);
  }
  if (length != 0) {
    timeoutTime = now + TIMEOUT;
    // Bytes available to read.
    length = socket->read_some(asio::buffer(bufferReceive), errorCode);
    if (!errorCode) {
      result = protocol->processReceiveBuffer(bufferReceive.data(), length);
      if (!result)
        return result;
    } else
      return ResultCode_t::READ_FAULT + errorCode.message() + endpoint;
  }
  if (protocol->hasTransmitBuffers()) {
    timeoutTime = now + TIMEOUT;
    length      = socket->write_some(protocol->getTransmitBuffers(), errorCode);
    if (errorCode == asio::error::would_block) {
      return ResultCode_t::INCOMPLETE;
    } else if (!errorCode) {
      if (protocol->updateTransmitBuffers(length)) {
        return ResultCode_t::INCOMPLETE;
      }
    } else
      return ResultCode_t::WRITE_FAULT + errorCode.message() + endpoint;
  }
  if (protocol->isDone()) {
    switch (protocol->getChangeRequest()) {
      case AppProtocol_t::HTTP:
        delete protocol;
        protocol = new HTTP::HTTP();
        return ResultCode_t::INCOMPLETE;
      case AppProtocol_t::WEBSOCKET:
        delete protocol;
        protocol = new WebSocket::WebSocket(gui);
        return ResultCode_t::INCOMPLETE;
      case AppProtocol_t::NONE:
        return ResultCode_t::SUCCESS;
      default:
        return ResultCode_t::INVALID_STATE +
               ("Connection AppProtocol change to: " +
                   std::to_string(
                       static_cast<uint8_t>(protocol->getChangeRequest())));
    }
  }
  if (now > timeoutTime && protocol->sendAliveCheck())
    return ResultCode_t::TIMEOUT;
  return ResultCode_t::NO_OPERATION;
}

/**
 * @brief Add a message to the protocol to transmit out if available
 * returns ResultCode_t::NOT_SUPPORTED if not compatible with the protocol
 *
 * @param msg to add
 * @return Result
 */
Result Connection::addMessage(const EBMessage_t & msg) {
  return protocol->addMessage(msg);
}

/**
 * @brief Stop the socket and free its memory
 *
 */
void Connection::stop() {
  if (socket != nullptr && socket->is_open()) {
    asio::error_code errorCode;
    socket->shutdown(asio::ip::tcp::socket::shutdown_both, errorCode);
    socket->close(errorCode);
  }
  delete socket;
  socket = nullptr;
  delete protocol;
  protocol = nullptr;
}

/**
 * @brief Get the endpoint of the request as a string
 *
 * @return const std::string & endpoint string
 */
const std::string & Connection::getEndpoint() const {
  return endpoint;
}

} // namespace Web