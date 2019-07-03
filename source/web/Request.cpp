#include "Request.h"

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
 * @return EBResult_t error code
 */
EBResult_t Request::parse(char * begin, char * end) {
  EBResult_t result = EBRESULT_SUCCESS;
  // For every character in the array, add it to the appropriate field based on
  // the current parsing state
  while (begin != end) {
    result = parse(*begin);
    if (EBRESULT_ERROR(result))
      return result;
    ++begin;
  }
  if (state == ParsingState_t::BODY && contentLength == body.size())
    return EBRESULT_SUCCESS;
  else if (body.size() > contentLength)
    return EBRESULT_BUFFER_OVERFLOW;
  else
    return EBRESULT_INCOMPLETE_OPERATION;
}

/**
 * @brief Parse a single character into the request
 *
 * @param c character to parse
 * @return EBResult_t error code
 */
EBResult_t Request::parse(char c) {
  EBResult_t result = EBRESULT_SUCCESS;
  switch (state) {
    case ParsingState_t::IDLE:
      state = ParsingState_t::METHOD;
      // Fall through
    case ParsingState_t::METHOD:
      if (c == ' ') {
        state       = ParsingState_t::URI;
        method.hash = Hash::calculateHash(method.string);
        result      = validateMethod();
        if (EBRESULT_ERROR(result))
          return result;
      } else
        method.string += c;
      break;
    case ParsingState_t::URI:
      if (c == ' ') {
        state  = ParsingState_t::HTTP_VERSION;
        result = decodeURI(uri.string);
        if (EBRESULT_ERROR(result))
          return result;
        uri.hash = Hash::calculateHash(uri.string);
      } else if (c == '?') {
        state  = ParsingState_t::QUERY_NAME;
        result = decodeURI(uri.string);
        if (EBRESULT_ERROR(result))
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
        if (EBRESULT_ERROR(result))
          return result;
        query.name.hash = Hash::calculateHash(query.name.string);
      } else if (c == ' ') {
        // Valueless query
        state  = ParsingState_t::HTTP_VERSION;
        result = decodeURI(query.name.string);
        if (EBRESULT_ERROR(result))
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
        if (EBRESULT_ERROR(result))
          return result;
        query.value.hash = Hash::calculateHash(query.value.string);
      } else if (c == '&') {
        state  = ParsingState_t::QUERY_NAME;
        result = decodeURI(query.value.string);
        if (EBRESULT_ERROR(result))
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
        if (EBRESULT_ERROR(result))
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
          contentLength = std::stoll(header.value.string);
          body.reserve(contentLength);
        } else if (header.name.hash == Hash::calculateHash("Connection")) {
          if (header.value.hash == Hash::calculateHash("keep-alive"))
            keepAlive = true;
          else if (header.value.hash == Hash::calculateHash("close"))
            keepAlive = false;
          else
            return EBRESULT_BAD_COMMAND;
        }
      } else if (c != '\r')
        header.value.string += c;
    } break;
    case ParsingState_t::BODY_NEWLINE:
      if (c == '\n') {
        state = ParsingState_t::BODY;
      } else
        return EBRESULT_BAD_COMMAND;
      break;
    case ParsingState_t::BODY:
      if (contentLength == 0) {
        // spdlog::debug("Tried to add '{}' (0x{:02X}) to body", *begin,
        // *begin);
        return EBRESULT_BAD_COMMAND;
      }
      body += c;
      break;
    default:
      return EBRESULT_BAD_COMMAND;
  }
  return EBRESULT_SUCCESS;
}

/**
 * @brief Checks the method is a valid HTTP request and supported
 *
 * @return EBResult_t error code
 */
EBResult_t Request::validateMethod() {
  switch (method.hash) {
    case Hash::calculateHash("GET"):
    case Hash::calculateHash("POST"):
      return EBRESULT_SUCCESS;
    case Hash::calculateHash("OPTIONS"):
    case Hash::calculateHash("HEAD"):
    case Hash::calculateHash("PUT"):
    case Hash::calculateHash("DELETE"):
    case Hash::calculateHash("TRACE"):
    case Hash::calculateHash("CONNECT"):
      return EBRESULT_NOT_SUPPORTED;
    default:
      return EBRESULT_BAD_COMMAND;
  }
}

/**
 * @brief Checks the HTTP version is valid and supported
 *
 * @return EBResult_t error code
 */
EBResult_t Request::validateHTTPVersion() {
  switch (httpVersion.hash) {
    case Hash::calculateHash("HTTP/1.0"):
    case Hash::calculateHash("HTTP/1.1"):
      return EBRESULT_SUCCESS;
    case Hash::calculateHash("HTTP/2.0"):
      return EBRESULT_VERSION_NOT_SUPPORTED;
    default:
      return EBRESULT_BAD_COMMAND;
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
 * @brief Get the endpoint of the request
 *
 * @return asio::ip::tcp::endpoint endpoint
 */
asio::ip::tcp::endpoint Request::getEndpoint() const {
  return endpoint;
}

/**
 * @brief Decode a URI into a string
 * Turns escape characters into their real characters
 *
 * @param uri string to read and overwrite
 * @return EBResult_t error code
 */
EBResult_t Request::decodeURI(std::string & uri) {
  EBResult_t result = EBRESULT_SUCCESS;
  size_t     length = uri.size();
  for (int i = 0; i < length; ++i) {
    switch (uri[i]) {
      case '%':
        // Next two letters are hex
        if (i + 2 < uri.size()) {
          uint32_t    value = 0;
          std::string hex   = uri.substr(i + 1, 2);
          result            = decodeHex(hex, value);
          if (EBRESULT_ERROR(result))
            return EBRESULT_INVALID_DATA;
          uri[i] = static_cast<char>(value);
          uri.erase(i + 1, 2);
          length -= 2;
        } else
          return EBRESULT_INVALID_DATA;
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
  return EBRESULT_SUCCESS;
}

/**
 * @brief Decode a string of hex characters (0-9, A-F, a-f) into a number
 * Up to 32bits or 8 characters
 *
 * @param hex to decode
 * @param value to return
 * @return EBResult_t error code
 */
EBResult_t Request::decodeHex(const std::string & hex, uint32_t & value) {
  if (hex.size() > 8)
    return EBRESULT_BUFFER_OVERFLOW;
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
      return EBRESULT_INVALID_DATA;
  }
  return EBRESULT_SUCCESS;
}

} // namespace Web