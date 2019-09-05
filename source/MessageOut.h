#ifndef _MESSAGE_OUT_H_
#define _MESSAGE_OUT_H_

#include <FruitBowl.h>
#include <rapidjson/document.h>

namespace Ehbanana {

class MessageOut {
public:
  MessageOut(const MessageOut &) = delete;
  MessageOut & operator=(const MessageOut &) = delete;

  MessageOut();
  ~MessageOut();

  Result setHref(const char * href);
  Result setProperty(const char * id, const char * name, const char * value);

  const std::string & getString(bool updateEnqueued = true);

  bool isEnqueued() const;

private:
  rapidjson::Document json;
  std::string         buf;

  bool enqueued = false;
};

} // namespace Ehbanana

#endif /* _MESSAGE_OUT_H_ */