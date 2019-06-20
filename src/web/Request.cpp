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
  queries.clear();
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
            state  = HTTP_VERSION;
            result = decodeURI(uri.string);
            if (!result)
              return result;
            uri.hash = Hash::calculateHash(uri.string);
          } else if (*begin == '?') {
            state  = QUERY_NAME;
            result = decodeURI(uri.string);
            if (!result)
              return result;
            uri.hash = Hash::calculateHash(uri.string);
          } else
            uri.string += *begin;
          ++begin;
        }
        break;
      case QUERY_NAME: {
        // If there are no queries or the last query is already complete, add a
        // new one
        if (queries.empty() || queries.back().value.hash != 0)
          queries.push_back(HeaderHash_t());
        HeaderHash_t & query = queries.back();
        while (begin != end && state == QUERY_NAME) {
          if (*begin == '=') {
            state  = QUERY_VALUE;
            result = decodeURI(query.name.string);
            if (!result)
              return result;
            query.name.hash = Hash::calculateHash(query.name.string);
          } else if (*begin == ' ') {
            // Valueless query
            state  = HTTP_VERSION;
            result = decodeURI(query.name.string);
            if (!result)
              return result;
            query.name.hash = Hash::calculateHash(query.name.string);
          } else
            query.name.string += *begin;
          ++begin;
        }
      } break;
      case QUERY_VALUE: {
        HeaderHash_t & query = queries.back();
        while (begin != end && state == QUERY_VALUE) {
          if (*begin == ' ') {
            state  = HTTP_VERSION;
            result = decodeURI(query.value.string);
            if (!result)
              return result;
            query.value.hash = Hash::calculateHash(query.value.string);
          } else if (*begin == '&') {
            state  = QUERY_NAME;
            result = decodeURI(query.value.string);
            if (!result)
              return result;
            query.value.hash = Hash::calculateHash(query.value.string);
          } else
            query.value.string += *begin;
          ++begin;
        }
      } break;
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

/**
 * @brief Get the method of the request
 *
 * @return const HashSet_t& method
 */
const HashSet_t & Request::getMethod() const {
  return method;
}

/**
 * @brief Get the URI of the request
 * Uniform resource identifier
 *
 * @return const HashSet_t& uri
 */
const HashSet_t & Request::getURI() const {
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
 * @param uri string to read and overwrite
 * @return Results::Result_t error code
 */
Results::Result_t Request::decodeURI(std::string & uri) {
  Results::Result_t result = Results::SUCCESS;
  size_t            length = uri.size();
  for (int i = 0; i < length; ++i) {
    switch (uri[i]) {
      case '%':
        // Next two letters are hex
        if (i + 2 < uri.size()) {
          uint32_t    value = 0;
          std::string hex   = uri.substr(i + 1, 2);
          result            = decodeHex(hex, value);
          if (!result)
            return Results::INVALID_DATA +
                   ("URI has invalid hex escape: %" + hex);
          uri[i] = static_cast<char>(value);
          uri.erase(i + 1, 2);
          length -= 2;
        } else
          return Results::INVALID_DATA + "URI has '%' but not enough character";
        break;
      case '+':
        // plus becomes space
        uri[i] = ' ';
        break;
      default:
        // Leave the character alone
        break;
    }
  }
  return Results::SUCCESS;
}

/**
 * @brief Decode a string of hex characters (0-9, A-F, a-f) into a number
 * Up to 32bits or 8 characters
 *
 * @param hex to decode
 * @param value to return
 * @return Results::Result_t error code
 */
Results::Result_t Request::decodeHex(
    const std::string & hex, uint32_t & value) {
  if (hex.size() > 8)
    return Results::BUFFER_OVERFLOW +
           ("Hex string is too long for 32bits: 0x\"" + hex + "\"");
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
      return Results::INVALID_DATA +
             ("Hex string contains non-hex characters: 0x\"" + hex + "\"");
  }
  return Results::SUCCESS;
}

} // namespace Web