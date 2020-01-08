#ifndef _WEB_HTTP_HTTP_H_
#define _WEB_HTTP_HTTP_H_

#include "Reply.h"
#include "Request.h"

#include "..\AppProtocol.h"

#include <string>

namespace Ehbanana {
namespace Web {
namespace HTTP {

class HTTP : public AppProtocol {
public:
  HTTP(const HTTP &) = delete;
  HTTP & operator=(const HTTP &) = delete;

  HTTP(Server * server);
  ~HTTP();

  void          processReceiveBuffer(const uint8_t * begin, size_t length);
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

  void handleRequest();
  void handleGET();
  void handlePOST();
  void handleUpgrade();

  enum class State_t : uint8_t {
    READING_HEADER,
    READING_BODY,
    READING_DONE,
    WRITING
  };

  State_t state = State_t::READING_HEADER;

  Request request;
  Reply   reply;
};

} // namespace HTTP
} // namespace Web
} // namespace Ehbanana

#endif /* _WEB_HTTP_HTTP_H_ */