#include "Connection.h"

#include "EhbananaLog.h"

#include "stdio.h"

namespace Ehbanana {
namespace Web {

/**
 * @brief Construct a new Connection:: Connection object
 *
 * @param socket to read from and write to
 * @param endpoint socket is connected to
 * @param now current timestamp
 * @param server parent to callback
 */
Connection::Connection(std::unique_ptr<Net::socket_t> socket,
    std::string endpoint, const timepoint_t<sysclk_t> & now, Server * server) :
  socket(std::move(socket)),
  endpoint(endpoint), server(server) {
  this->timeoutTime = now + TIMEOUT;

  this->socket->non_blocking(true);
  this->socket->set_option(Net::socket_t::keep_alive(true));
  protocol = std::make_unique<HTTP::HTTP>(server);
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
 * state = BUSY if more operations are required
 * state = IDLE if nothing happenend this update
 * state = DONE if the connection was idle for too long or closed
 *
 * @param now current timestamp
 * @throw std::exception Thrown on failure
 */
void Connection::update(const timepoint_t<sysclk_t> & now) {
  state = State_t::IDLE;
  asio::error_code errorCode;
  size_t           length = socket->available(errorCode);
  if (errorCode)
    throw std::exception(
        ("Checking socket's availability: " + errorCode.message()).c_str());

  if (length != 0) {
    state       = State_t::BUSY;
    timeoutTime = now + TIMEOUT;
    // Bytes available to read.
    length = socket->read_some(asio::buffer(bufferReceive), errorCode);
    if (!errorCode) {
      try {
        protocol->processReceiveBuffer(bufferReceive.data(), length);
      } catch (const std::exception & e) {
        std::string filename =
            std::to_string(now.time_since_epoch().count()) + ".tmp";
        FILE *  file;
        errno_t err = fopen_s(&file, filename.c_str(), "wb");
        if (err)
          throw std::exception("Failed to open RX buffer file");
        fwrite(bufferReceive.data(), 1, length, file);
        error("Exception occurred whilst processing RX buffer. Buffer saved "
              "to: " +
              filename);
        throw e;
      }
    } else {
      throw std::exception(("Reading socket: " + errorCode.message()).c_str());
    }
  }

  if (protocol->hasTransmitBuffers()) {
    state       = State_t::BUSY;
    timeoutTime = now + TIMEOUT;
    length      = socket->write_some(protocol->getTransmitBuffers(), errorCode);
    if (!errorCode) {
      protocol->updateTransmitBuffers(length);
    } else if (errorCode != asio::error::would_block) {
      throw std::exception(("Writing socket: " + errorCode.message()).c_str());
    }
  }

  if (protocol->isDone()) {
    switch (protocol->getChangeRequest()) {
      case AppProtocol_t::HTTP:
        protocol = std::make_unique<HTTP::HTTP>(server);
        state    = State_t::BUSY;
        info(endpoint + ": Changing to HTTP");
        break;
      case AppProtocol_t::WEBSOCKET:
        protocol = std::make_unique<WebSocket::WebSocket>(server);
        state    = State_t::BUSY;
        info(endpoint + ": Changing to WebSocket");
        break;
      case AppProtocol_t::NONE:
        state = State_t::DONE;
        break;
      default:
        throw std::exception((
            "Connection AppProtocol change to: " +
            std::to_string(static_cast<uint8_t>(protocol->getChangeRequest())))
                                 .c_str());
    }
  }
  if (now > timeoutTime && protocol->sendAliveCheck())
    state = State_t::DONE;
}

/**
 * @brief Stop the socket and free its memory
 *
 * @throw std::exception Thrown on failure
 */
void Connection::stop() {
  if (socket != nullptr && socket->is_open()) {
    socket->shutdown(Net::socket_t::shutdown_both);
    socket->close();
  }
}

/**
 * @brief Get the string representation of the connection
 *
 * @return const std::string& ip address and port
 */
const std::string & Connection::toString() const {
  return endpoint;
}

/**
 * @brief Get the state of the connection
 *
 * @return State_t
 */
Connection::State_t Connection::getState() const {
  return state;
}

} // namespace Web
} // namespace Ehbanana