#include "Connection.h"

#include <spdlog/spdlog.h>

namespace Web {

/**
 * @brief Construct a new Connection:: Connection object
 *
 * @param socket to read from and write to
 * @param endpoint socket is connected to
 * @param requestHandler to use to process requests
 */
Connection::Connection(asio::ip::tcp::socket * socket,
    asio::ip::tcp::endpoint endpoint, RequestHandler * requestHandler) :
  request(endpoint) {
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
 * Returns EBRESULT_INCOMPLETE_OPERATION if more operations are required
 * Returns EBRESULT_NO_OPERATION if nothing happenend this update
 * Returns EBRESULT_TIMEOUT if the connection was idle for too long
 *
 * @param now current timestamp
 * @return EBResultMsg_t error code
 */
EBResultMsg_t Connection::update(
    const std::chrono::time_point<std::chrono::system_clock> & now) {
  EBResultMsg_t result = EBResult::SUCCESS;
  switch (state) {
    case State_t::IDLE:
      reply.stockReply(HTTPStatus_t::OK);
      state       = State_t::READING;
      timeoutTime = now + TIMEOUT;
      // Fall through
    case State_t::READING:
      result = read();
      if (result == EBResult::SUCCESS)
        state = State_t::READING_DONE;
      else if (result == EBResult::INCOMPLETE_OPERATION)
        return result;
      else if (result == EBResult::NO_OPERATION)
        return (now < timeoutTime) ? result : EBResult::TIMEOUT;
      else {
        // An error occurred while reading the request, generate the appropriate
        // stock reply
        spdlog::warn(result);
        reply.stockReply(result);
        state = State_t::WRITING;
      }
      break;
    case State_t::READING_DONE:
      result = requestHandler->handle(request, reply);
      if (!result) {
        // An error occurred while reading the request, generate the appropriate
        // stock reply
        spdlog::warn(result);
        reply.stockReply(result);
      }

      reply.setKeepAlive(request.isKeepAlive());
      state = State_t::WRITING;
      break;
    case State_t::WRITING:
      result = write();
      if (!result)
        return result;
      state = State_t::WRITING_DONE;
      break;
    case State_t::WRITING_DONE:
      if (request.isKeepAlive()) {
        reply.reset();
        request.reset();
        state = State_t::IDLE;
      } else
        state = State_t::COMPLETE;
      break;
    case State_t::COMPLETE:
      return EBResult::SUCCESS;
    default:
      return EBResult::INVALID_STATE + "During connection update";
  }
  return EBResult::NO_OPERATION;
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
 * Returns EBRESULT_INCOMPLETE_OPERATION if more reading is required
 * Returns EBRESULT_NO_OPERATION if nothing was read and nothing was expected
 *
 * @return EBResultMsg_t error code
 */
EBResultMsg_t Connection::read() {
  asio::error_code errorCode;
  size_t           length = socket->read_some(asio::buffer(buffer), errorCode);
  if (!errorCode) {
    return request.parse(buffer.data(), buffer.data() + length);
  } else if (errorCode == asio::error::would_block)
    return request.isParsing() ? EBResult::INCOMPLETE_OPERATION
                               : EBResult::NO_OPERATION;
  return EBResult::ASIO_READ_FAULT;
}

/**
 * @brief Write data to the socket from a reply
 *
 * Returns EBRESULT_INCOMPLETE_OPERATION if more writing is required
 *
 * @return EBResult_t error code
 */
EBResultMsg_t Connection::write() {
  asio::error_code errorCode;
  size_t           length = socket->write_some(reply.getBuffers(), errorCode);
  if (reply.updateBuffers(length)) {
    if (!errorCode || errorCode == asio::error::would_block)
      return EBResult::INCOMPLETE_OPERATION;
  } else if (!errorCode)
    return EBResult::SUCCESS;
  return EBResult::ASIO_WRITE_FAULT;
}

} // namespace Web