#include "Request.h"

#include <spdlog/spdlog.h>
#include <sstream>

namespace Web {

/**
 * @brief Construct a new Request:: Request object
 *
 * @param endpoint that sourced this request
 */
Request::Request(asio::ip::tcp::endpoint endpoint) {
  this->endpoint = endpoint;
}

/**
 * @brief Reset all fields to their default state
 *
 */
void Request::reset() {
  state         = ParsingState_t::IDLE;
  method        = Hash();
  uri           = Hash();
  httpVersion   = Hash();
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
 * @return Result error code
 */
Result Request::parse(char * begin, char * end) {
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
  if (state == ParsingState_t::BODY && contentLength == body.size())
    return ResultCode_t::SUCCESS;
  else if (body.size() > contentLength)
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
Result Request::parse(char c) {
  Result result;
  switch (state) {
    case ParsingState_t::IDLE:
      state = ParsingState_t::METHOD;
      // Fall through
    case ParsingState_t::METHOD:
      if (c == ' ') {
        state  = ParsingState_t::URI;
        result = validateMethod();
        if (!result)
          return result + "Validating request method";
      } else
        method.add(c);
      break;
    case ParsingState_t::URI:
      if (c == ' ') {
        state  = ParsingState_t::HTTP_VERSION;
        result = decodeURI(uri);
        if (!result)
          return result + "Decoding request URI";
      } else if (c == '?') {
        state  = ParsingState_t::QUERY_NAME;
        result = decodeURI(uri);
        if (!result)
          return result + "Decoding request URI";
      } else
        uri.add(c);
      break;
    case ParsingState_t::QUERY_NAME: {
      // If there are no queries or the last query is already complete, add a
      // new one
      if (queries.empty() || queries.back().value.isDone())
        queries.push_back(HeaderHash_t());
      HeaderHash_t & query = queries.back();
      if (c == '=') {
        state  = ParsingState_t::QUERY_VALUE;
        result = decodeURI(query.name);
        if (!result)
          return result + "Decoding request URI of query name";
      } else if (c == ' ') {
        // Valueless query
        state  = ParsingState_t::HTTP_VERSION;
        result = decodeURI(query.name);
        if (!result)
          return result + "Decoding request URI of query name";
      } else
        query.name.add(c);

    } break;
    case ParsingState_t::QUERY_VALUE: {
      HeaderHash_t & query = queries.back();
      if (c == ' ') {
        state  = ParsingState_t::HTTP_VERSION;
        result = decodeURI(query.value);
        query.value.setDone(true);
        if (!result)
          return result + "Decoding request URI of query value";
      } else if (c == '&') {
        state  = ParsingState_t::QUERY_NAME;
        result = decodeURI(query.value);
        if (!result)
          return result + "Decoding request URI of query value";
      } else
        query.value.add(c);
    } break;
    case ParsingState_t::HTTP_VERSION:
      if (c == '\n') {
        state  = ParsingState_t::HEADER_NAME;
        result = validateHTTPVersion();
        if (!result)
          return result + "Validating request HTTP version";
      } else if (c != '\r')
        httpVersion.add(c);
      break;
    case ParsingState_t::HEADER_NAME: {
      if (c == '\r') {
        state = ParsingState_t::BODY_NEWLINE;
        break;
      }
      // If there are no headers or the last header is already complete, add a
      // new one
      if (headers.empty() || headers.back().value.isDone())
        headers.push_back(HeaderHash_t());
      HeaderHash_t & header = headers.back();
      if (c == ' ') {
        state = ParsingState_t::HEADER_VALUE;
      } else if (c != ':')
        header.name.add(c);
    } break;
    case ParsingState_t::HEADER_VALUE: {
      HeaderHash_t & header = headers.back();
      if (c == '\n') {
        state = ParsingState_t::HEADER_NAME;
        header.value.setDone(true);
        if (header.name.get() == Hash::calculateHash("Content-Length")) {
          contentLength =
              static_cast<size_t>(std::stoll(header.value.getString()));
          body.reserve(contentLength);
        } else if (header.name.get() == Hash::calculateHash("Connection")) {
          if (header.value.get() == Hash::calculateHash("keep-alive"))
            keepAlive = true;
          else if (header.value.get() == Hash::calculateHash("close"))
            keepAlive = false;
          else
            return ResultCode_t::BAD_COMMAND +
                   ("Request's Connection is " + header.value.getString());
        }
      } else if (c != '\r')
        header.value.add(c);
    } break;
    case ParsingState_t::BODY_NEWLINE:
      if (c == '\n') {
        state = ParsingState_t::BODY;
      } else
        return ResultCode_t::BAD_COMMAND +
               "Request is missing a blank new line after header";
      break;
    case ParsingState_t::BODY:
      if (contentLength == 0) {
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
  return state != ParsingState_t::IDLE &&
         (state != ParsingState_t::BODY || contentLength != body.size());
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
 * @brief Get the endpoint of the request as a string
 *
 * @return const std::string & endpoint string
 */
std::string Request::getEndpointString() const {
  std::ostringstream os;
  os << endpoint;
  return os.str();
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

} // namespace Web