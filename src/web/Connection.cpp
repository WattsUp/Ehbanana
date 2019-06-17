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
 *
 * @return Results::Result_t error code
 */
Results::Result_t Connection::update() {
  Results::Result_t result = Results::SUCCESS;
  switch (state) {
    case IDLE:
      state = READING;
      // Fall through
    case READING:
      result = read();
      if (result == Results::INCOMPLETE_OPERATION)
        return Results::INCOMPLETE_OPERATION;
      else if (!result)
        return result;
      else
        state = READING_DONE;
      // Fall through
    case READING_DONE:
      state = WRITING;
      // Fall through
    case WRITING:
      result = write();
      if (result == Results::INCOMPLETE_OPERATION)
        return Results::INCOMPLETE_OPERATION;
      else if (!result)
        return result;
      else
        state = WRITING_DONE;
      // Fall through
    case WRITING_DONE:
      state = COMPLETE;
      // Fall through
    case COMPLETE:
      return Results::SUCCESS;
    default:
      return Results::BAD_COMMAND;
  }
  return Results::BAD_COMMAND;
}

/**
 * @brief Stop the socket and free its memory
 *
 */
void Connection::stop() {
  if (socket != nullptr)
    socket->close();
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
 *
 * @return Results::Result_t error code
 */
Results::Result_t Connection::read() {
  asio::error_code errorCode;
  size_t           length = socket->read_some(asio::buffer(buffer), errorCode);
  if (!errorCode) {
    return request.parse(buffer.data(), buffer.data() + length);
  } else if (errorCode == asio::error::would_block)
    return Results::INCOMPLETE_OPERATION;
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
  return Results::NOT_SUPPORTED;
}

} // namespace Web