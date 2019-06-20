#include "Connection.h"
#include "spdlog/spdlog.h"

namespace Web {

/**
 * @brief Construct a new Connection:: Connection object
 *
 * @param socket to read from and write to
 * @param endpoint socket is connected to
 * @param requestHandler to use to process requests
 */
Connection::Connection(asio::ip::tcp::socket * socket,
    asio::ip::tcp::endpoint endpoint, RequestHandler * requestHandler) {
  this->socket         = socket;
  this->endpoint       = endpoint;
  this->requestHandler = requestHandler;
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
 * @brief Update the current operation, read or write
 *
 * Returns Results::INCOMPLETE_OPERATION if more operations are required
 * Returns Results::NO_OPERATION if nothing happenend this update
 * Returns Results::TIMEOUT if the connection was idle for too long
 *
 * @param now current timestamp
 * @return Results::Result_t error code
 */
Results::Result_t Connection::update(
    const std::chrono::time_point<std::chrono::system_clock> & now) {
  Results::Result_t result = Results::SUCCESS;
  switch (state) {
    case IDLE:
      reply.stockReply(HTTPStatus::OK);
      state       = READING;
      timeoutTime = now + TIMEOUT;
      // Fall through
    case READING:
      result = read();
      if (result == Results::SUCCESS)
        state = READING_DONE;
      else if (result == Results::INCOMPLETE_OPERATION)
        return result;
      else if (result == Results::NO_OPERATION)
        return (now < timeoutTime)
                   ? result
                   : Results::TIMEOUT + "Connection was idle for 60s";
      else {
        // An error occurred while reading the request, generate the appropriate
        // stock reply
        spdlog::warn(result);
        reply.stockReply(result);
        state = WRITING;
      }
      break;
    case READING_DONE:
      result = requestHandler->handle(request, reply);
      if (!result) {
        // An error occurred while reading the request, generate the appropriate
        // stock reply
        spdlog::warn(result);
        reply.stockReply(result);
      }

      reply.setKeepAlive(request.isKeepAlive());
      state = WRITING;
      break;
    case WRITING:
      result = write();
      if (result == Results::SUCCESS)
        state = WRITING_DONE;
      else
        return result;
      break;
    case WRITING_DONE:
      if (request.isKeepAlive()) {
        reply.reset();
        request.reset();
        state = IDLE;
      } else
        state = COMPLETE;
      break;
    case COMPLETE:
      return Results::SUCCESS;
    default:
      return Results::BAD_COMMAND + "Connection update in unknown state";
  }
  return Results::NO_OPERATION;
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
}

/**
 * @brief Get the endpoint of the connection
 *
 * @return asio::ip::tcp::endpoint endpoint
 */
asio::ip::tcp::endpoint Connection::getEndpoint() {
  return endpoint;
}

/**
 * @brief Read data from the socket into a request and process if reading is
 * complete
 *
 * Returns Results::INCOMPLETE_OPERATION if more reading is required
 * Returns Results::NO_OPERATION if nothing was read and nothing was expected
 *
 * @return Results::Result_t error code
 */
Results::Result_t Connection::read() {
  asio::error_code errorCode;
  size_t           length = socket->read_some(asio::buffer(buffer), errorCode);
  if (!errorCode) {
    return request.parse(buffer.data(), buffer.data() + length);
  } else if (errorCode == asio::error::would_block)
    return request.isParsing() ? Results::INCOMPLETE_OPERATION
                               : Results::NO_OPERATION;
  return Results::READ_FAULT +
         ("Reading socket failed with ASIO error: " + errorCode.message());
}

/**
 * @brief Write data to the socket from a reply
 *
 * Returns Results::INCOMPLETE_OPERATION if more writing is required
 *
 * @return Results::Result_t error code
 */
Results::Result_t Connection::write() {
  asio::error_code errorCode;
  size_t           length = socket->write_some(reply.getBuffers(), errorCode);
  if (reply.updateBuffers(length)) {
    if (!errorCode || errorCode == asio::error::would_block) {
      return Results::INCOMPLETE_OPERATION;
    }
  } else if (!errorCode) {
    return Results::SUCCESS;
  }

  return Results::READ_FAULT +
         ("Writing socket failed with ASIO error: " + errorCode.message());
}

} // namespace Web