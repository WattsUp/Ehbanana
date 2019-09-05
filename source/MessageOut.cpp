#include "MessageOut.h"

#include <rapidjson/prettywriter.h>

namespace Ehbanana {

/**
 * @brief Construct a new Message Out:: Message Out object
 *
 */
MessageOut::MessageOut() {
  json.SetObject();
  json.AddMember(
      "href", rapidjson::Value("", json.GetAllocator()), json.GetAllocator());
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
 * @brief Set the href (webpage) of the message
 * "" will alert all pages
 *
 * @param href string
 * @return Result
 */
Result MessageOut::setHref(const char * href) {
  json["href"].SetString(href, json.GetAllocator());
  return ResultCode_t::SUCCESS;
}

/**
 * @brief Set the property of an HTML element by ID
 *
 * @param id of the element
 * @param name of the property
 * @param value of the property
 * @return Result
 */
Result MessageOut::setProperty(
    const char * id, const char * name, const char * value) {
  if (!json["elements"].HasMember(id)) {
    rapidjson::Value element;
    element.SetObject();
    json["elements"].AddMember(rapidjson::Value(id, json.GetAllocator()),
        element, json.GetAllocator());
  }
  if (!json["elements"][id].HasMember(name)) {
    json["elements"][id].AddMember(rapidjson::Value(name, json.GetAllocator()),
        rapidjson::Value(value, json.GetAllocator()), json.GetAllocator());
  } else {
    json["elements"][id][name].SetString(value, json.GetAllocator());
  }
  rapidjson::StringBuffer                          sb;
  rapidjson::PrettyWriter<rapidjson::StringBuffer> writer(sb);
  json.Accept(writer);
  buf = sb.GetString();
  return ResultCode_t::SUCCESS;
}

/**
 * @brief Get the string representation of the message
 *
 * @param updateEnqueued will set enqueued upon returning
 * @return const std::string& JSON string
 */
const std::string & MessageOut::getString(bool updateEnqueued) {
  enqueued = enqueued || updateEnqueued;
  rapidjson::StringBuffer                          sb;
  rapidjson::PrettyWriter<rapidjson::StringBuffer> writer(sb);
  json.Accept(writer);
  buf = sb.GetString();
  return buf;
}

/**
 * @brief Check if the message has been enqueued already
 *
 * @return true if it has been enqueued
 * @return false if it has not been enqueued
 */
bool MessageOut::isEnqueued() const {
  return enqueued;
}

} // namespace Ehbanana