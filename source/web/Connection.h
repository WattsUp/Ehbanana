#ifndef _WEB_CONNECTION_H_
#define _WEB_CONNECTION_H_

#include <FruitBowl.h>
#include <asio.hpp>

#include <array>
#include <chrono>
#include <stdint.h>
#include <string>

namespace Web {

class Connection {
public:
  enum class Protocol_t : uint8_t { NONE, HTTP, WEBSOCKET, PENDING_WEBSOCKET };

  Connection(const Connection &) = delete;
  Connection & operator=(const Connection &) = delete;

  /**
   * @brief Construct a new Connection object
   *
   * @param socket to read from and write to
   * @param endpoint  socket is connected to
   */
  Connection(asio::ip::tcp::socket * socket, std::string endpoint) :
    socket(socket), endpoint(endpoint) {
    socket->non_blocking(true);

    asio::socket_base::keep_alive option(true);
    socket->set_option(option);
  }

  /**
   * @brief Destroy the Connection object
   */
  ~Connection() {}

  /**
   * @brief Update the connection
   *
   * Returns ResultCode_t::INCOMPLETE if more operations are required
   * Returns ResultCode_t::NO_OPERATION if nothing happenend this update
   * Returns ResultCode_t::TIMEOUT if the connection was idle for too long
   *
   * @param now current timestamp
   * @return Result error code
   */
  virtual Result update(
      const std::chrono::time_point<std::chrono::system_clock> & now) = 0;

  /**
   * @brief Stop the socket and free its memory
   *
   */
  void stop() {
    if (socket != nullptr && socket->is_open()) {
      asio::error_code errorCode;
      socket->shutdown(asio::ip::tcp::socket::shutdown_both, errorCode);
      socket->close(errorCode);
    }
    delete socket;
    socket = nullptr;
  }

  /**
   * @brief Get the Endpoint as a string
   *
   * @return const std::string&
   */
  const std::string & getEndpoint() const {
    return endpoint;
  }

  /**
   * @brief Get the Protocol of the connection
   *
   * @return const Protocol_t
   */
  const Protocol_t getProtocol() const {
    return protocol;
  }

  /**
   * @brief Get the Socket object
   *
   * @return const asio::ip::tcp::socket*
   */
  asio::ip::tcp::socket * getSocket() const {
    return socket;
  }

protected:
  asio::ip::tcp::socket * socket;
  std::string             endpoint;

  Protocol_t protocol = Protocol_t::NONE;

  std::array<char, 8192> buffer;

  std::chrono::time_point<std::chrono::system_clock> timeoutTime;

  const std::chrono::seconds TIMEOUT {60};
};

} // namespace Web

#endif /* _WEB_CONNECTION_H_ */