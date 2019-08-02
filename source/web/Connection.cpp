#include "Connection.h"

#include <spdlog/spdlog.h>
#include <sstream>

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
 * Returns ResultCode_t::INCOMPLETE if more operations are required
 * Returns ResultCode_t::NO_OPERATION if nothing happenend this update
 * Returns ResultCode_t::TIMEOUT if the connection was idle for too long
 *
 * @param now current timestamp
 * @return Result error code
 */
Result Connection::update(
    const std::chrono::time_point<std::chrono::system_clock> & now) {
  Result result;
  switch (state) {
    case State_t::IDLE:
      reply.stockReply(HTTPStatus_t::OK);
      state       = State_t::READING;
      timeoutTime = now + TIMEOUT;
      // Fall through
    case State_t::READING:
      result = read();
      if (result)
        state = State_t::READING_DONE;
      else if (result == ResultCode_t::INCOMPLETE)
        return ResultCode_t::INCOMPLETE;
      else if (result == ResultCode_t::NO_OPERATION)
        return (now < timeoutTime)
                   ? ResultCode_t::NO_OPERATION
                   : ResultCode_t::TIMEOUT + "Reading connection";
      else {
        // An error occurred while reading the request, generate the appropriate
        // stock reply
        spdlog::warn(result.getMessage());
        reply.stockReply(result);
        state = State_t::WRITING;
      }
      break;
    case State_t::READING_DONE:
      result = requestHandler->handle(request, reply);
      if (!result) {
        // An error occurred while reading the request, generate the appropriate
        // stock reply
        spdlog::warn(result.getMessage());
        reply.stockReply(result);
      }

      reply.setKeepAlive(request.isKeepAlive());
      state = State_t::WRITING;
      break;
    case State_t::WRITING:
      result = write();
      if (!result)
        return result + "Writing connection";
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
      return ResultCode_t::SUCCESS;
    default:
      return ResultCode_t::INVALID_STATE + "During connection update";
  }
  return ResultCode_t::NO_OPERATION;
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
 * @brief Get the endpoint of the request as a string
 *
 * @return const std::string & endpoint string
 */
std::string Connection::getEndpointString() const {
  std::ostringstream os;
  os << endpoint;
  return os.str();
}

/**
 * @brief Read data from the socket into a request and process if reading is
 * complete
 *
 * Returns ResultCode_t::INCOMPLETE if more reading is required
 * Returns ResultCode_t::NO_OPERATION if nothing was read and nothing was
 * expected
 *
 * @return Result error code
 */
Result Connection::read() {
  asio::error_code errorCode;
  size_t           length = socket->read_some(asio::buffer(buffer), errorCode);
  if (!errorCode) {
    return request.parse(buffer.data(), buffer.data() + length);
  } else if (errorCode == asio::error::would_block)
    return request.isParsing() ? ResultCode_t::INCOMPLETE
                               : ResultCode_t::NO_OPERATION;
  return ResultCode_t::READ_FAULT;
}

/**
 * @brief Write data to the socket from a reply
 *
 * Returns ResultCode_t::INCOMPLETE if more writing is required
 *
 * @return Result error code
 */
Result Connection::write() {
  asio::error_code errorCode;
  size_t           length = socket->write_some(reply.getBuffers(), errorCode);
  if (reply.updateBuffers(length)) {
    if (!errorCode || errorCode == asio::error::would_block)
      return ResultCode_t::INCOMPLETE;
  } else if (!errorCode)
    return ResultCode_t::SUCCESS;
  return ResultCode_t::WRITE_FAULT;
}

} // namespace Web