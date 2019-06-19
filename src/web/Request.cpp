#include "Request.h"

#include "spdlog/spdlog.h"

namespace Web {

/**
 * @brief Construct a new Request:: Request object
 *
 */
Request::Request() {}

/**
 * @brief Reset all fields to their default state
 *
 */
void Request::reset() {
  state         = IDLE;
  method        = {0, ""};
  uri           = {0, ""};
  httpVersion   = {0, ""};
  contentLength = 0;
  keepAlive     = false;
  body.clear();
  headers.clear();
}

/**
 * @brief Parse a string and add its contents to the request
 *
 * The string may be a portion of the entire request
 *
 * @param begin character pointer
 * @param end character pointer
 * @return Results::Result_t error code
 */
Results::Result_t Request::parse(char * begin, char * end) {
  Results::Result_t result = Results::SUCCESS;
  // For every character in the array, add it to the appropriate field based on
  // the current parsing state
  while (begin != end) {
    switch (state) {
      case IDLE:
        state = METHOD;
        // Fall through
      case METHOD:
        while (begin != end && state == METHOD) {
          if (*begin == ' ') {
            state       = URI;
            method.hash = Hash::calculateHash(method.string);
            result      = validateMethod();
            if (!result)
              return result;
          } else
            method.string += *begin;
          ++begin;
        }
        break;
      case URI:
        while (begin != end && state == URI) {
          if (*begin == ' ') {
            state    = HTTP_VERSION;
            uri.hash = Hash::calculateHash(uri.string);
            spdlog::debug("URI: 0x{:08X} \"{}\"", uri.hash, uri.string);
          } else
            uri.string += *begin;
          ++begin;
        }
        break;
      case HTTP_VERSION:
        while (begin != end && state == HTTP_VERSION) {
          if (*begin == '\n') {
            state            = HEADER_NAME;
            httpVersion.hash = Hash::calculateHash(httpVersion.string);
            result           = validateHTTPVersion();
            if (!result)
              return result;
          } else if (*begin != '\r')
            httpVersion.string += *begin;
          ++begin;
        }
        break;
      case HEADER_NAME: {
        if (*begin == '\r') {
          state = BODY_NEWLINE;
          ++begin;
          break;
        }
        // If there are no headers or the last header is already complete, add a
        // new one
        if (headers.empty() || headers.back().value.hash != 0)
          headers.push_back(HeaderHash_t());
        HeaderHash_t & header = headers.back();
        while (begin != end && state == HEADER_NAME) {
          if (*begin == ' ') {
            state            = HEADER_VALUE;
            header.name.hash = Hash::calculateHash(header.name.string);
          } else if (*begin != ':')
            header.name.string += *begin;
          ++begin;
        }
      } break;
      case HEADER_VALUE: {
        HeaderHash_t & header = headers.back();
        while (begin != end && state == HEADER_VALUE) {
          if (*begin == '\n') {
            state             = HEADER_NAME;
            header.value.hash = Hash::calculateHash(header.value.string);
            if (header.name.hash == Hash::calculateHash("Content-Length")) {
              contentLength = std::stoll(header.value.string);
              body.reserve(contentLength);
            } else if (header.name.hash == Hash::calculateHash("Connection")) {
              if (header.value.hash == Hash::calculateHash("keep-alive"))
                keepAlive = true;
              else if (header.value.hash == Hash::calculateHash("close"))
                keepAlive = false;
              else
                return Results::BAD_COMMAND +
                       ("HTTP request unknown Connection value: " +
                           header.value.string);
            }
          } else if (*begin != '\r')
            header.value.string += *begin;
          ++begin;
        }
      } break;
      case BODY_NEWLINE:
        if (*begin == '\n') {
          state = BODY;
          ++begin;
        } else
          return Results::BAD_COMMAND +
                 "HTTP request unknown during BODY_NEWLINE";
        break;
      case BODY:
        if (contentLength == 0) {
          spdlog::debug("Tried to add '{}' (0x{:02X}) to body", *begin, *begin);
          return Results::BAD_COMMAND +
                 "HTTP request zero content length but has a body";
        }
        while (begin != end) {
          body += *begin;
          ++begin;
        }
        break;
      default:
        return Results::BAD_COMMAND + "Unknown HTTP request parsing state";
    }
  }
  if (state == BODY && contentLength == body.size())
    return Results::SUCCESS;
  else if (body.size() > contentLength)
    return Results::BUFFER_OVERFLOW +
           "HTTP request content length does not match body length";
  else
    return Results::INCOMPLETE_OPERATION;
}

/**
 * @brief Checks the method is a valid HTTP request and supported
 *
 * @return Results::Result_t error code
 */
Results::Result_t Request::validateMethod() {
  switch (method.hash) {
    case Hash::calculateHash("GET"):
    case Hash::calculateHash("POST"):
      return Results::SUCCESS;
    case Hash::calculateHash("OPTIONS"):
    case Hash::calculateHash("HEAD"):
    case Hash::calculateHash("PUT"):
    case Hash::calculateHash("DELETE"):
    case Hash::calculateHash("TRACE"):
    case Hash::calculateHash("CONNECT"):
      return Results::NOT_SUPPORTED +
             ("Unsupported HTTP method: \"" + method.string + "\"");
    default:
      return Results::BAD_COMMAND +
             ("Unknown method: \"" + method.string + "\"");
  }
}

/**
 * @brief Checks the HTTP version is valid and supported
 *
 * @return Results::Result_t error code
 */
Results::Result_t Request::validateHTTPVersion() {
  switch (httpVersion.hash) {
    case Hash::calculateHash("HTTP/1.0"):
    case Hash::calculateHash("HTTP/1.1"):
      return Results::SUCCESS;
    case Hash::calculateHash("HTTP/2.0"):
      return Results::VERSION_NOT_SUPPORTED +
             "HTTP/2.0 is not supported on this server";
    default:
      return Results::BAD_COMMAND +
             ("Unknown HTTP version: \"" + method.string + "\"");
  }
}

/**
 * @brief Get the keep alive status
 *
 * @return true if keep alive was requested
 * @return false otherwise
 */
bool Request::isKeepAlive() {
  return keepAlive;
}

/**
 * @brief Get the state of parsing
 *
 * @return true if a request is partially parsed
 * @return false otherwise
 */
bool Request::isParsing() {
  return state != IDLE && (state != BODY || contentLength != body.size());
}

} // namespace Web