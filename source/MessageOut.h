#ifndef _EHBANANA_MESSAGE_OUT_H_
#define _EHBANANA_MESSAGE_OUT_H_

#include <rapidjson/document.h>
#include <rapidjson/stringbuffer.h>

namespace Ehbanana {

class MessageOut {
public:
  MessageOut(const MessageOut &) = delete;
  MessageOut & operator=(const MessageOut &) = delete;

  MessageOut(const char * uri);
  ~MessageOut();

  template <typename T>
  void add(const char * id, const char * property, const T & value) {
    if (!json["elements"].HasMember(id))
      json["elements"].AddMember(rapidjson::Value(id, json.GetAllocator()),
          rapidjson::Value().SetObject(), json.GetAllocator());

    json["elements"][id].RemoveMember(property);
    json["elements"][id].AddMember(
        rapidjson::Value(property, json.GetAllocator()),
        rapidjson::Value(value), json.GetAllocator());
  }

  void add(const char * id, const char * property, const char * value) {
    if (!json["elements"].HasMember(id))
      json["elements"].AddMember(rapidjson::Value(id, json.GetAllocator()),
          rapidjson::Value().SetObject(), json.GetAllocator());

    json["elements"][id].RemoveMember(property);
    json["elements"][id].AddMember(
        rapidjson::Value(property, json.GetAllocator()),
        rapidjson::Value(value, json.GetAllocator()), json.GetAllocator());
  }

  const char * getString(bool forceRewrite = false);
  size_t       size(bool forceRewrite = false);

private:
  rapidjson::Document     json;
  rapidjson::StringBuffer buffer;
};

} // namespace Ehbanana

#endif /* _EHBANANA_MESSAGE_OUT_H_ */