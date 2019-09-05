#include "Request.h"

#include <spdlog/spdlog.h>
#include <sstream>

namespace Web {
namespace HTTP {

/**
 * @brief Construct a new Request:: Request object
 *
 */
Request::Request() {}

/**
 * @brief Destroy the Request:: Request object
 *
 */
Request::~Request() {}

/**
 * @brief Parse a string and add its contents to the request
 *
 * The string may be a fragment of the entire request
 *
 * @param begin character pointer
 * @param end character pointer
 * @return Result error code
 */
Result Request::parse(const uint8_t * begin, const uint8_t * end) {
  Result result;
  // For every character in the array, add it to the appropriate field based on
  // the current parsing state
  while (begin != end) {
    result = parse(*begin);
    if (!result) {
      return result + "Parsing request character";
    }
    ++begin;
  }
  if (state == State_t::BODY && headers.getContentLength() == body.size())
    return ResultCode_t::SUCCESS;
  else if (body.size() > headers.getContentLength())
    return ResultCode_t::BUFFER_OVERFLOW +
           "Request body size is longer than content length";
  else
    return ResultCode_t::INCOMPLETE;
}

/**
 * @brief Parse a single character into the request
 *
 * @param c character to parse
 * @return Result error code
 */
Result Request::parse(uint8_t c) {
  Result result;
  switch (state) {
    case State_t::IDLE:
      state = State_t::METHOD;
      // Fall through
    case State_t::METHOD:
      if (c == ' ') {
        state  = State_t::URI;
        result = validateMethod();
        if (!result)
          return result + "Validating request method";
      } else
        method.add(c);
      break;
    case State_t::URI:
      if (c == ' ') {
        state  = State_t::HTTP_VERSION;
        result = decodeURI(uri);
        if (!result)
          return result + "Decoding request URI";
      } else if (c == '?') {
        state  = State_t::QUERY_NAME;
        result = decodeURI(uri);
        if (!result)
          return result + "Decoding request URI";
      } else
        uri.add(c);
      break;
    case State_t::QUERY_NAME: {
      // If there are no queries or the last query is already complete, add a
      // new one
      if (queries.empty() || queries.back().value.isDone())
        queries.push_back(HeaderHash_t());
      HeaderHash_t & query = queries.back();
      if (c == '=') {
        state  = State_t::QUERY_VALUE;
        result = decodeURI(query.name);
        if (!result)
          return result + "Decoding request URI of query name";
      } else if (c == ' ') {
        // Valueless query
        state  = State_t::HTTP_VERSION;
        result = decodeURI(query.name);
        if (!result)
          return result + "Decoding request URI of query name";
      } else
        query.name.add(c);

    } break;
    case State_t::QUERY_VALUE: {
      HeaderHash_t & query = queries.back();
      if (c == ' ') {
        state  = State_t::HTTP_VERSION;
        result = decodeURI(query.value);
        query.value.setDone(true);
        if (!result)
          return result + "Decoding request URI of query value";
      } else if (c == '&') {
        state  = State_t::QUERY_NAME;
        result = decodeURI(query.value);
        if (!result)
          return result + "Decoding request URI of query value";
      } else
        query.value.add(c);
    } break;
    case State_t::HTTP_VERSION:
      if (c == '\n') {
        state  = State_t::HEADER_NAME;
        result = validateHTTPVersion();
        if (!result)
          return result + "Validating request HTTP version";
      } else if (c != '\r')
        httpVersion.add(c);
      break;
    case State_t::HEADER_NAME: {
      if (c == '\r') {
        state = State_t::BODY_NEWLINE;
        break;
      }
      // If there are no headers or the last header is already complete, add a
      // new one
      if (c == ' ') {
        state = State_t::HEADER_VALUE;
      } else if (c != ':')
        currentHeader.name.add(c);
    } break;
    case State_t::HEADER_VALUE: {
      if (c == '\n') {
        state  = State_t::HEADER_NAME;
        result = headers.addHeader(currentHeader);
        if (result == ResultCode_t::UNKNOWN_HASH)
          spdlog::warn(result.getMessage());
        else if (!result)
          return result + "Adding request header";
        currentHeader = HeaderHash_t();
      } else if (c != '\r')
        currentHeader.value.add(c);
    } break;
    case State_t::BODY_NEWLINE:
      if (c == '\n') {
        state = State_t::BODY;
        body.reserve(headers.getContentLength());
      } else
        return ResultCode_t::BAD_COMMAND +
               "Request is missing a blank new line after header";
      break;
    case State_t::BODY:
      if (headers.getContentLength() == 0) {
        spdlog::debug("Tried to add '{}' (0x{:02X}) to body", c, c);
        return ResultCode_t::BAD_COMMAND +
               "Request has body but zero content length";
      }
      body += c;
      break;
    default:
      return ResultCode_t::INVALID_STATE +
             ("Request parsing is in state #" + static_cast<uint8_t>(state));
  }
  return ResultCode_t::SUCCESS;
}

/**
 * @brief Checks the method is a valid HTTP request and supported
 *
 * @return Result error code
 */
Result Request::validateMethod() {
  switch (method.get()) {
    case Hash::calculateHash("GET"):
    case Hash::calculateHash("POST"):
      return ResultCode_t::SUCCESS;
    case Hash::calculateHash("OPTIONS"):
    case Hash::calculateHash("HEAD"):
    case Hash::calculateHash("PUT"):
    case Hash::calculateHash("DELETE"):
    case Hash::calculateHash("TRACE"):
    case Hash::calculateHash("CONNECT"):
      return ResultCode_t::NOT_SUPPORTED + "Request method";
    default:
      return ResultCode_t::UNKNOWN_HASH +
             ("Request method: " + method.getString());
  }
}

/**
 * @brief Checks the HTTP version is valid and supported
 *
 * @return Result error code
 */
Result Request::validateHTTPVersion() {
  switch (httpVersion.get()) {
    case Hash::calculateHash("HTTP/1.0"):
    case Hash::calculateHash("HTTP/1.1"):
      return ResultCode_t::SUCCESS;
    case Hash::calculateHash("HTTP/2.0"):
      return ResultCode_t::NOT_SUPPORTED + "Request HTTP/2.0";
    default:
      return ResultCode_t::UNKNOWN_HASH +
             ("Request HTTP version: " + httpVersion.getString());
  }
}

/**
 * @brief Get the headers of the request
 *
 * @return const RequestHeaders&
 */
const RequestHeaders & Request::getHeaders() const {
  return headers;
}

/**
 * @brief Get the state of parsing
 *
 * @return true if a request is partially parsed
 * @return false otherwise
 */
bool Request::isParsing() {
  return state != State_t::IDLE &&
         (state != State_t::BODY || headers.getContentLength() != body.size());
}

/**
 * @brief Get the method of the request
 *
 * @return const Hash& method
 */
const Hash & Request::getMethod() const {
  return method;
}

/**
 * @brief Get the URI of the request
 * Uniform resource identifier
 *
 * @return const Hash& uri
 */
const Hash & Request::getURI() const {
  return uri;
}

/**
 * @brief Get the queries of the request's URI
 * Uniform resource identifier
 *
 * @return const std::vector<HeaderHash_t>& queries
 */
const std::vector<HeaderHash_t> & Request::getQueries() const {
  return queries;
}

/**
 * @brief Decode a URI into a string
 * Turns escape characters into their real characters
 *
 * @param uriString to read and overwrite
 * @return Result error code
 */
Result Request::decodeURI(Hash & uriHash) {
  Result              result;
  const std::string & string = uriHash.getString();
  size_t              i      = 0;
  Hash                uriDecoded;
  while (i < string.length()) {
    switch (string[i]) {
      case '%':
        // Next two letters are hex
        if (i + 2 >= string.length())
          return ResultCode_t::INVALID_DATA +
                 "URI has '%' but not enough characters";
        else {
          uint32_t    value = 0;
          std::string hex   = string.substr(i, 2);
          i += 2;

          result = decodeHex(hex, value);
          if (!result)
            return result + ("Decoding hex: " + hex);
          uriDecoded.add(static_cast<char>(value));
        }
        break;
      case '+':
        // plus becomes space
        uriDecoded.add(' ');
        ++i;
        break;
      default:
        // Leave the character alone
        uriDecoded.add(string[i]);
        ++i;
        break;
    }
  }
  uriHash = uriDecoded;
  return ResultCode_t::SUCCESS;
}

/**
 * @brief Decode a string of hex characters (0-9, A-F, a-f) into a number
 * Up to 32bits or 8 characters
 *
 * @param hex to decode
 * @param value to return
 * @return Result error code
 */
Result Request::decodeHex(const std::string & hex, uint32_t & value) {
  if (hex.size() > 8)
    return ResultCode_t::BUFFER_OVERFLOW + "Too many characters";
  value = 0;
  for (char c : hex) {
    value = value << 4;
    if (c >= '0' && c <= '9')
      value |= (c - '0');
    else if (c >= 'a' && c <= 'f')
      value |= (c - 'a' + 10);
    else if (c >= 'A' && c <= 'F')
      value |= (c - 'A' + 10);
    else
      return ResultCode_t::INVALID_DATA + ("Character is not hex: " + c);
  }
  return ResultCode_t::SUCCESS;
}

} // namespace HTTP
} // namespace Web