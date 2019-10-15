#include "Root.h"

#include <algorithm/sha1.hpp>
#include <base64.h>

namespace GUI {

/**
 * @brief Construct a new Root:: Root object
 *
 * @param gui
 */
Root::Root(EBGUI_t gui) : Page(gui, "") {}

/**
 * @brief Destroy the Root:: Root object
 *
 */
Root::~Root() {}

/**
 * @brief Handle when the page first loads
 * Likely send initial values
 *
 * @return Result
 */
Result Root::onLoad() {
  try {
    createNewEBMessage();

    std::string temp = "I have a";
    messageSetProp("fruit-checkbox0", "checked", appleSelected);
    messageSetProp("fruit-checkbox1", "checked", bananaSelected);
    messageSetProp("fruit-checkbox2", "checked", orangeSelected);
    if (appleSelected)
      temp += "n apple";
    if (bananaSelected)
      temp += " banana";
    if (orangeSelected)
      temp += " orange";
    messageSetProp("fruit-check-out", "innerHTML", temp);

    enqueueEBMessage();
  } catch (const std::exception & e) {
    return ResultCode_t::EXCEPTION_OCCURRED + e.what();
  }

  return sendUpdate();
}

/**
 * @brief Send an update to the page
 * Likely send real time values
 *
 * @return Result
 */
Result Root::sendUpdate() {
  try {
    createNewEBMessage();

    messageSetProp("stream-out", "innerHTML", std::to_string(rand() & 0xFF));

    enqueueEBMessage();
  } catch (const std::exception & e) {
    return ResultCode_t::EXCEPTION_OCCURRED + e.what();
  }
  return ResultCode_t::SUCCESS;
}

/**
 * @brief Handle the input from the GUI
 *
 * @param msg to handle
 * @return Result
 */
Result Root::handleInput(const EBMessage_t & msg) {
  try {
    createNewEBMessage();

    switch (msg.id.get()) {
      case Hash::calculateHash("body"):
        return onLoad();
      case Hash::calculateHash("text-in"): {
        std::string temp(msg.value.getString());
        std::reverse(temp.begin(), temp.end());
        messageSetProp("text-out", "innerHTML", temp);
      } break;
      case Hash::calculateHash("password-in"): {
        digestpp::sha1 sha;
        sha.absorb(msg.value.getString());
        uint8_t buf[20];
        sha.digest(buf, 20);
        messageSetProp("password-out", "innerHTML", base64_encode(buf, 20));
      } break;
      case Hash::calculateHash("life-in"):
        messageSetProp("life-out", "innerHTML", "42");
        break;
      case Hash::calculateHash("fruit"):
        if (msg.value.get() == Hash::calculateHash("Banana"))
          messageSetProp(
              "fruit-out", "innerHTML", "You have chosen <i>wisely</i>");
        else if (msg.value.get() == Hash::calculateHash("Apple"))
          messageSetProp(
              "fruit-out", "innerHTML", "You have chosen <i>wrong</i>");
        else
          messageSetProp(
              "fruit-out", "innerHTML", "You have chosen <i>poorly</i>");
        break;
      case Hash::calculateHash("fruit-checkbox0"): {
        std::string temp = "I have a";
        appleSelected    = msg.value.get() == Hash::calculateHash("true");
        if (appleSelected)
          temp += "n apple";
        if (bananaSelected)
          temp += " banana";
        if (orangeSelected)
          temp += " orange";
        messageSetProp("fruit-check-out", "innerHTML", temp);
      } break;
      case Hash::calculateHash("fruit-checkbox1"): {
        std::string temp = "I have a";
        bananaSelected   = msg.value.get() == Hash::calculateHash("true");
        if (appleSelected)
          temp += "n apple";
        if (bananaSelected)
          temp += " banana";
        if (orangeSelected)
          temp += " orange";
        messageSetProp("fruit-check-out", "innerHTML", temp);
      } break;
      case Hash::calculateHash("fruit-checkbox2"): {
        std::string temp = "I have a";
        orangeSelected   = msg.value.get() == Hash::calculateHash("true");
        if (appleSelected)
          temp += "n apple";
        if (bananaSelected)
          temp += " banana";
        if (orangeSelected)
          temp += " orange";
        messageSetProp("fruit-check-out", "innerHTML", temp);
      } break;
      case Hash::calculateHash("color-in"):
        messageSetProp("color-out", "value", msg.value.getString());
        break;
      case Hash::calculateHash("date-in"):
        messageSetProp("date-out", "value", msg.value.getString());
        break;
      case Hash::calculateHash("datetime-in"):
        messageSetProp("datetime-out", "value", msg.value.getString());
        break;
      case Hash::calculateHash("email-in"):
        messageSetProp("email-out", "innerHTML", msg.value.getString());
        break;
      case Hash::calculateHash("file-in"): {
        if (msg.file == nullptr)
          return ResultCode_t::INVALID_DATA;
        digestpp::sha1 sha;
        int            i;
        while ((i = fgetc(msg.file)) != EOF) {
          char c = static_cast<char>(i);
          sha.absorb(&c, 1);
        }
        fclose(msg.file);
        uint8_t shaBuf[20];
        sha.digest(shaBuf, 20);
        messageSetProp("file-out", "innerHTML", base64_encode(shaBuf, 20));
      } break;
      case Hash::calculateHash("month-in"):
        messageSetProp("month-out", "value", msg.value.getString());
        break;
      case Hash::calculateHash("number-in"):
        messageSetProp("number-out", "innerHTML", msg.value.getString());
        break;
      case Hash::calculateHash("range-in"):
        messageSetProp("range-out", "innerHTML", msg.value.getString());
        break;
      case Hash::calculateHash("tel-in"):
        messageSetProp("tel-out", "innerHTML", msg.value.getString());
        break;
      case Hash::calculateHash("time-in"):
        messageSetProp("time-out", "value", msg.value.getString());
        break;
      case Hash::calculateHash("url-in"):
        messageSetProp("url-out", "innerHTML", msg.value.getString());
        break;
      default:
        return ResultCode_t::UNKNOWN_HASH;
    }

    enqueueEBMessage();
  } catch (const std::exception & e) {
    return ResultCode_t::EXCEPTION_OCCURRED + e.what();
  }
  return ResultCode_t::SUCCESS;
}

} // namespace GUI