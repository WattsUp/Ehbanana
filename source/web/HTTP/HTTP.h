#ifndef _WEB_HTTP_HTTP_H_
#define _WEB_HTTP_HTTP_H_

#include "Reply.h"
#include "Request.h"

#include "..\AppProtocol.h"

#include <string>

namespace Web {
namespace HTTP {

class HTTP : public AppProtocol {
public:
  HTTP(const HTTP &) = delete;
  HTTP & operator=(const HTTP &) = delete;

  HTTP();
  ~HTTP();

  Result        processReceiveBuffer(const uint8_t * begin, size_t length);
  bool          updateTransmitBuffers(size_t bytesWritten);
  bool          isDone();
  AppProtocol_t getChangeRequest();

  /**
   * @brief Set the root directory to serve http from
   *
   * @param httpRoot relative or absolute path
   */
  static void setRoot(const std::string & httpRoot) {
    root() = httpRoot;
  }

private:
  /**
   * @brief Get the root of the http directory
   *
   * @return std::string&
   */
  static std::string & root() {
    static std::string httpRoot;
    return httpRoot;
  }

  Result handleRequest();
  Result handleGET();
  Result handlePOST();
  Result handleUpgrade();

  enum class State_t : uint8_t {
    READING,
    READING_DONE,
    WRITING,
    WRITING_DONE,
    COMPLETE
  };

  State_t state = State_t::READING;

  Request request;
  Reply   reply;
};

} // namespace HTTP
} // namespace Web

#endif /* _WEB_HTTP_HTTP_H_ */