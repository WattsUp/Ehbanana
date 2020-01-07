#include "Request.h"

#include "EhbananaLog.h"

namespace Ehbanana {
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
 * @return bool true if complete, false otherwise
 * @throw std::exception Thrown on failure
 */
bool Request::parse(const uint8_t * begin, const uint8_t * end) {
  // For every character in the array, add it to the appropriate field based on
  // the current parsing state
  while (begin != end) {
    parse(*begin);
    ++begin;
  }
  if (state == State_t::BODY && headers.getContentLength() == body.size())
    return true;
  else if (body.size() > headers.getContentLength())
    throw std::overflow_error("Received body is larger than header states");
  return false;
}

/**
 * @brief Parse a single character into the request
 *
 * @param c character to parse
 * @throw std::exception Thrown on failure
 */
void Request::parse(uint8_t c) {
  switch (state) {
    case State_t::IDLE:
      state = State_t::METHOD;
      // Fall through
    case State_t::METHOD:
      if (c == ' ') {
        state = State_t::URI;
        validateMethod();
      } else
        method.add(c);
      break;
    case State_t::URI:
      if (c == ' ') {
        state = State_t::HTTP_VERSION;
        uri   = decodeURI(uri);
      } else if (c == '?') {
        state = State_t::QUERY_NAME;
        uri   = decodeURI(uri);
      } else
        uri += c;
      break;
    case State_t::QUERY_NAME:
      if (c == '=') {
        state             = State_t::QUERY_VALUE;
        currentQuery.name = decodeURI(currentQuery.name);
      } else if (c == '&') {
        // Valueless query
        state             = State_t::QUERY_NAME;
        currentQuery.name = decodeURI(currentQuery.name);
        queries.push_back(currentQuery);
        currentQuery = Query_t();
      } else if (c == ' ') {
        // Valueless query
        state             = State_t::HTTP_VERSION;
        currentQuery.name = decodeURI(currentQuery.name);
        queries.push_back(currentQuery);
      } else {
        currentQuery.name += c;
      }
      break;
    case State_t::QUERY_VALUE:
      if (c == ' ') {
        state              = State_t::HTTP_VERSION;
        currentQuery.value = decodeURI(currentQuery.value);
        queries.push_back(currentQuery);
      } else if (c == '&') {
        state              = State_t::QUERY_NAME;
        currentQuery.value = decodeURI(currentQuery.value);
        queries.push_back(currentQuery);
        currentQuery = Query_t();
      } else {
        currentQuery.value += c;
      }
      break;
    case State_t::HTTP_VERSION:
      if (c == '\n') {
        state = State_t::HEADER_NAME;
        validateHTTPVersion();
      } else if (c != '\r')
        httpVersion.add(c);
      break;
    case State_t::HEADER_NAME: {
      if (c == '\r') {
        state = State_t::BODY_NEWLINE;
        break;
      }
      if (c == ' ') {
        state = State_t::HEADER_VALUE;
      } else if (c != ':')
        currentHeader.name.add(c);
    } break;
    case State_t::HEADER_VALUE: {
      if (c == '\n') {
        state = State_t::HEADER_NAME;
        headers.addHeader(currentHeader);
        currentHeader = HeaderHash_t();
      } else if (c != '\r')
        currentHeader.value.add(c);
    } break;
    case State_t::BODY_NEWLINE:
      if (c == '\n') {
        state = State_t::BODY;
        body.reserve(headers.getContentLength());
      } else {
        throw std::exception("Request is missing new line after headers");
      }
      break;
    case State_t::BODY:
      if (headers.getContentLength() == 0) {
        log(EBLogLevel_t::EB_DEBUG, "Tried to add '%c' (0x%02X) to body", c, c);
        throw std::overflow_error("Request has body but zero content length");
      }
      body += c;
      break;
    default:
      throw std::exception(("Request parsing is in state #" +
                            std::to_string(static_cast<uint8_t>(state)))
                               .c_str());
  }
}

/**
 * @brief Checks the method is a valid HTTP request and supported
 *
 * @throw std::exception Thrown on failure
 */
void Request::validateMethod() {
  switch (method.get()) {
    case Hash::calculateHash("GET"):
    case Hash::calculateHash("POST"):
      return;
    case Hash::calculateHash("OPTIONS"):
    case Hash::calculateHash("HEAD"):
    case Hash::calculateHash("PUT"):
    case Hash::calculateHash("DELETE"):
    case Hash::calculateHash("TRACE"):
    case Hash::calculateHash("CONNECT"):
      throw std::exception("HTTP method is not supported");
    default:
      throw std::exception("HTTP method is not recognized");
  }
}

/**
 * @brief Checks the HTTP version is valid and supported
 *
 * @throw std::exception Thrown on failure
 */
void Request::validateHTTPVersion() {
  switch (httpVersion.get()) {
    case Hash::calculateHash("HTTP/1.0"):
    case Hash::calculateHash("HTTP/1.1"):
      return;
    case Hash::calculateHash("HTTP/2.0"):
      throw std::exception("HTTP version is not supported");
    default:
      throw std::exception("HTTP version is not recognized");
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
 * @return HashValue_t
 */
HashValue_t Request::getMethodHash() const {
  return method.get();
}

/**
 * @brief Get the URI of the request
 * Uniform resource identifier
 *
 * @return const std::string& uri
 */
const std::string & Request::getURI() const {
  return uri;
}

/**
 * @brief Get the body of the request
 *
 * @return const std::string&
 */
const std::string & Request::getBody() const {
  return body;
}

/**
 * @brief Get the queries of the request's URI
 * Uniform resource identifier
 *
 * @return const std::list<HeaderHash_t>& queries
 */
const std::list<Request::Query_t> & Request::getQueries() const {
  return queries;
}

/**
 * @brief Decode a URI into a string
 * Turns escape characters into their real characters
 *
 * @param uriString to read
 * @return std::string decoded
 * @throw std::exception Thrown on failure
 */
std::string Request::decodeURI(const std::string & uriString) {
  size_t      i = 0;
  std::string uriDecoded;
  while (i < uriString.length()) {
    switch (uriString[i]) {
      case '%':
        // Next two letters are hex
        if (i + 2 >= uriString.length())
          throw std::exception("URI has '%' but not enough characters");
        else {
          uint32_t    value = 0;
          std::string hex   = uriString.substr(i, 2);
          i += 2;

          value = decodeHex(hex);
          uriDecoded += static_cast<char>(value);
        }
        break;
      case '+':
        // plus becomes space
        uriDecoded += ' ';
        ++i;
        break;
      default:
        // Leave the character alone
        uriDecoded += uriString[i];
        ++i;
        break;
    }
  }
  return uriDecoded;
}

/**
 * @brief Decode a string of hex characters (0-9, A-F, a-f) into a number
 * Up to 32bits or 8 characters
 *
 * @param hex to decode
 * @return uint32_t value
 * @throw std::exception Thrown on failure
 */
uint32_t Request::decodeHex(const std::string & hex) {
  if (hex.size() > 8)
    std::exception("Too many characeters to decode hex string");
  uint32_t value = 0;
  for (char c : hex) {
    value = value << 4;
    if (c >= '0' && c <= '9')
      value |= (c - '0');
    else if (c >= 'a' && c <= 'f')
      value |= (c - 'a' + 10);
    else if (c >= 'A' && c <= 'F')
      value |= (c - 'A' + 10);
    else
      throw std::exception("Character is not hex");
  }
  return value;
}

} // namespace HTTP
} // namespace Web
} // namespace Ehbanana