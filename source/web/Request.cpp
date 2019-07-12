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
 * @return EBResultMsg_t error code
 */
EBResultMsg_t Request::parse(char * begin, char * end) {
  EBResultMsg_t result = EBResult::SUCCESS;
  // For every character in the array, add it to the appropriate field based on
  // the current parsing state
  while (begin != end) {
    result = parse(*begin);
    if (!result) {
      return result;
    }
    ++begin;
  }
  if (state == ParsingState_t::BODY && contentLength == body.size())
    return EBResult::SUCCESS;
  else if (body.size() > contentLength)
    return EBResult::BUFFER_OVERFLOW +
           "Request body size is longer than content length";
  else
    return EBResult::INCOMPLETE_OPERATION;
}

/**
 * @brief Parse a single character into the request
 *
 * @param c character to parse
 * @return EBResultMsg_t error code
 */
EBResultMsg_t Request::parse(char c) {
  EBResultMsg_t result = EBResult::SUCCESS;
  switch (state) {
    case ParsingState_t::IDLE:
      state = ParsingState_t::METHOD;
      // Fall through
    case ParsingState_t::METHOD:
      if (c == ' ') {
        state       = ParsingState_t::URI;
        method.hash = Hash::calculateHash(method.string);
        result      = validateMethod();
        if (!result)
          return result;
      } else
        method.string += c;
      break;
    case ParsingState_t::URI:
      if (c == ' ') {
        state  = ParsingState_t::HTTP_VERSION;
        result = decodeURI(uri.string);
        if (!result)
          return result;
        uri.hash = Hash::calculateHash(uri.string);
      } else if (c == '?') {
        state  = ParsingState_t::QUERY_NAME;
        result = decodeURI(uri.string);
        if (!result)
          return result;
        uri.hash = Hash::calculateHash(uri.string);
      } else
        uri.string += c;
      break;
    case ParsingState_t::QUERY_NAME: {
      // If there are no queries or the last query is already complete, add a
      // new one
      if (queries.empty() || queries.back().value.hash != 0)
        queries.push_back(HeaderHash_t());
      HeaderHash_t & query = queries.back();
      if (c == '=') {
        state  = ParsingState_t::QUERY_VALUE;
        result = decodeURI(query.name.string);
        if (!result)
          return result;
        query.name.hash = Hash::calculateHash(query.name.string);
      } else if (c == ' ') {
        // Valueless query
        state  = ParsingState_t::HTTP_VERSION;
        result = decodeURI(query.name.string);
        if (!result)
          return result;
        query.name.hash = Hash::calculateHash(query.name.string);
      } else
        query.name.string += c;

    } break;
    case ParsingState_t::QUERY_VALUE: {
      HeaderHash_t & query = queries.back();
      if (c == ' ') {
        state  = ParsingState_t::HTTP_VERSION;
        result = decodeURI(query.value.string);
        if (!result)
          return result;
        query.value.hash = Hash::calculateHash(query.value.string);
      } else if (c == '&') {
        state  = ParsingState_t::QUERY_NAME;
        result = decodeURI(query.value.string);
        if (!result)
          return result;
        query.value.hash = Hash::calculateHash(query.value.string);
      } else
        query.value.string += c;
    } break;
    case ParsingState_t::HTTP_VERSION:
      if (c == '\n') {
        state            = ParsingState_t::HEADER_NAME;
        httpVersion.hash = Hash::calculateHash(httpVersion.string);
        result           = validateHTTPVersion();
        if (!result)
          return result;
      } else if (c != '\r')
        httpVersion.string += c;
      break;
    case ParsingState_t::HEADER_NAME: {
      if (c == '\r') {
        state = ParsingState_t::BODY_NEWLINE;
        break;
      }
      // If there are no headers or the last header is already complete, add a
      // new one
      if (headers.empty() || headers.back().value.hash != 0)
        headers.push_back(HeaderHash_t());
      HeaderHash_t & header = headers.back();
      if (c == ' ') {
        state            = ParsingState_t::HEADER_VALUE;
        header.name.hash = Hash::calculateHash(header.name.string);
      } else if (c != ':')
        header.name.string += c;
    } break;
    case ParsingState_t::HEADER_VALUE: {
      HeaderHash_t & header = headers.back();
      if (c == '\n') {
        state             = ParsingState_t::HEADER_NAME;
        header.value.hash = Hash::calculateHash(header.value.string);
        if (header.name.hash == Hash::calculateHash("Content-Length")) {
          contentLength = static_cast<size_t>(std::stoll(header.value.string));
          body.reserve(contentLength);
        } else if (header.name.hash == Hash::calculateHash("Connection")) {
          if (header.value.hash == Hash::calculateHash("keep-alive"))
            keepAlive = true;
          else if (header.value.hash == Hash::calculateHash("close"))
            keepAlive = false;
          else
            return EBResult::BAD_COMMAND +
                   ("Request's Connection is " + header.value.string);
        }
      } else if (c != '\r')
        header.value.string += c;
    } break;
    case ParsingState_t::BODY_NEWLINE:
      if (c == '\n') {
        state = ParsingState_t::BODY;
      } else
        return EBResult::BAD_COMMAND +
               "Request is missing a blank new line after header";
      break;
    case ParsingState_t::BODY:
      if (contentLength == 0) {
        spdlog::debug("Tried to add '{}' (0x{:02X}) to body", c, c);
        return EBResult::BAD_COMMAND +
               "Request has body but zero content length";
      }
      body += c;
      break;
    default:
      return EBResult::INVALID_STATE +
             ("Request parsing is in state #" + static_cast<uint8_t>(state));
  }
  return EBResult::SUCCESS;
}

/**
 * @brief Checks the method is a valid HTTP request and supported
 *
 * @return EBResultMsg_t error code
 */
EBResultMsg_t Request::validateMethod() {
  switch (method.hash) {
    case Hash::calculateHash("GET"):
    case Hash::calculateHash("POST"):
      return EBResult::SUCCESS;
    case Hash::calculateHash("OPTIONS"):
    case Hash::calculateHash("HEAD"):
    case Hash::calculateHash("PUT"):
    case Hash::calculateHash("DELETE"):
    case Hash::calculateHash("TRACE"):
    case Hash::calculateHash("CONNECT"):
      return EBResult::NOT_SUPPORTED + "Request method";
    default:
      return EBResult::UNKNOWN_HASH + ("Request method: " + method.string);
  }
}

/**
 * @brief Checks the HTTP version is valid and supported
 *
 * @return EBResultMsg_t error code
 */
EBResultMsg_t Request::validateHTTPVersion() {
  switch (httpVersion.hash) {
    case Hash::calculateHash("HTTP/1.0"):
    case Hash::calculateHash("HTTP/1.1"):
      return EBResult::SUCCESS;
    case Hash::calculateHash("HTTP/2.0"):
      return EBResult::VERSION_NOT_SUPPORTED + "Request HTTP/2.0";
    default:
      return EBResult::UNKNOWN_HASH + ("Request HTTP version: " + httpVersion.string);
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
 * @return EBResultMsg_t error code
 */
EBResultMsg_t Request::decodeURI(std::string & uriString) {
  EBResultMsg_t result = EBResult::SUCCESS;
  size_t        length = uriString.size();
  for (size_t i = 0; i < length; ++i) {
    switch (uriString[i]) {
      case '%':
        // Next two letters are hex
        if (i + 2 < uriString.size()) {
          uint32_t    value = 0;
          std::string hex   = uriString.substr(i + 1, 2);
          result            = decodeHex(hex, value);
          if (!result)
            return result;
          uriString[i] = static_cast<char>(value);
          uriString.erase(i + 1, 2);
          length -= 2;
        } else
          return EBResult::INVALID_DATA + "URI has '%' but not enough characters";
        break;
      case '+':
        // plus becomes space
        uriString[i] = ' ';
        break;
      default:
        // Leave the character alone
        break;
    }
  }
  return EBResult::SUCCESS;
}

/**
 * @brief Decode a string of hex characters (0-9, A-F, a-f) into a number
 * Up to 32bits or 8 characters
 *
 * @param hex to decode
 * @param value to return
 * @return EBResultMsg_t error code
 */
EBResultMsg_t Request::decodeHex(const std::string & hex, uint32_t & value) {
  if (hex.size() > 8)
    return EBResult::BUFFER_OVERFLOW + "Too many characters";
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
      return EBResult::INVALID_DATA + ("Character is not hex: " + c);
  }
  return EBResult::SUCCESS;
}

} // namespace Web