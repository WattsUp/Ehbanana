#include "MessageOut.h"

#include "EhbananaLog.h"

#include <rapidjson/writer.h>

namespace Ehbanana {

/**
 * @brief Construct a new Message Out:: Message Out object
 *
 * @param uri of the target page, nullptr will broadcast to al
 */
MessageOut::MessageOut(const char * uri) {
  json.SetObject();
  if (uri == nullptr)
    json.AddMember("href", "", json.GetAllocator());
  else
    json.AddMember("href", rapidjson::Value(uri, json.GetAllocator()),
        json.GetAllocator());
  rapidjson::Value elements;
  elements.SetObject();
  json.AddMember("elements", elements, json.GetAllocator());
}

/**
 * @brief Destroy the Message Out:: Message Out object
 *
 */
MessageOut::~MessageOut() {}

/**
 * @brief Get the string representation of the message
 *
 * @param forceRewrite will recrete the string if true
 * @return const char*
 */
const char * MessageOut::getString(bool forceRewrite) {
  if (forceRewrite || buffer.GetSize() == 0) {
    rapidjson::Writer<rapidjson::StringBuffer> writer(buffer);
    json.Accept(writer);
  }
  return buffer.GetString();
}

/**
 * @brief Get the size of the string representation of the message
 *
 * @param forceRewrite will recrete the string if true
 * @return size_t
 */
size_t MessageOut::size(bool forceRewrite) {
  if (forceRewrite || buffer.GetSize() == 0) {
    rapidjson::Writer<rapidjson::StringBuffer> writer(buffer);
    json.Accept(writer);
  }
  return buffer.GetSize();
}

} // namespace Ehbanana