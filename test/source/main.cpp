#include <Ehbanana.h>
#include <algorithm/sha1.hpp>
#include <base64.h>
#include <chrono>
#include <thread>

bool appleSelected  = false;
bool bananaSelected = false;
bool orangeSelected = false;

/**
 * @brief Handle an input message
 *
 * @param msg
 * @return Result
 */
Result handleInput(const EBMessage_t & msg) {
  Result result;
  result = EBMessageOutCreate(msg.gui);
  if (!result)
    return result;

  result = EBMessageOutSetHref(msg.gui, msg.href.getString());
  if (!result)
    return result;

  EBLogInfo(msg.href.getString() + "|" + msg.id.getString() + "|" +
            msg.value.getString());

  switch (msg.id.get()) {
    case Hash::calculateHash("text-in"): {
      std::string temp(msg.value.getString());
      std::reverse(temp.begin(), temp.end());
      result = EBMessageOutSetProp(msg.gui, "text-out", "innerHTML", temp);
    } break;
    case Hash::calculateHash("password-in"): {
      digestpp::sha1 sha;
      sha.absorb(msg.value.getString());
      uint8_t buf[20];
      sha.digest(buf, 20);
      result = EBMessageOutSetProp(
          msg.gui, "password-out", "innerHTML", base64_encode(buf, 20));
    } break;
    case Hash::calculateHash("life-in"):
      result = EBMessageOutSetProp(msg.gui, "life-out", "innerHTML", "42");
      break;
    case Hash::calculateHash("fruit"):
      if (msg.value.get() == Hash::calculateHash("Banana"))
        result = EBMessageOutSetProp(
            msg.gui, "fruit-out", "innerHTML", "You have chosen <i>wisely</i>");
      else if (msg.value.get() == Hash::calculateHash("Apple"))
        result = EBMessageOutSetProp(
            msg.gui, "fruit-out", "innerHTML", "You have chosen <i>wrong</i>");
      else
        result = EBMessageOutSetProp(
            msg.gui, "fruit-out", "innerHTML", "You have chosen <i>poorly</i>");
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
      result =
          EBMessageOutSetProp(msg.gui, "fruit-check-out", "innerHTML", temp);
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
      result =
          EBMessageOutSetProp(msg.gui, "fruit-check-out", "innerHTML", temp);
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
      result =
          EBMessageOutSetProp(msg.gui, "fruit-check-out", "innerHTML", temp);
    } break;
    case Hash::calculateHash("color-in"):
      result = EBMessageOutSetProp(
          msg.gui, "color-out", "value", msg.value.getString());
      break;
    case Hash::calculateHash("date-in"):
      result = EBMessageOutSetProp(
          msg.gui, "date-out", "value", msg.value.getString());
      break;
    case Hash::calculateHash("datetime-in"):
      result = EBMessageOutSetProp(
          msg.gui, "datetime-out", "value", msg.value.getString());
      break;
    case Hash::calculateHash("email-in"):
      result = EBMessageOutSetProp(
          msg.gui, "email-out", "innerHTML", msg.value.getString());
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
      result = EBMessageOutSetProp(
          msg.gui, "file-out", "innerHTML", base64_encode(shaBuf, 20));
    } break;
    case Hash::calculateHash("month-in"):
      result = EBMessageOutSetProp(
          msg.gui, "month-out", "value", msg.value.getString());
      break;
    case Hash::calculateHash("number-in"):
      result = EBMessageOutSetProp(
          msg.gui, "number-out", "innerHTML", msg.value.getString());
      break;
    case Hash::calculateHash("range-in"):
      result = EBMessageOutSetProp(
          msg.gui, "range-out", "innerHTML", msg.value.getString());
      break;
    case Hash::calculateHash("tel-in"):
      result = EBMessageOutSetProp(
          msg.gui, "tel-out", "innerHTML", msg.value.getString());
      break;
    case Hash::calculateHash("time-in"):
      result = EBMessageOutSetProp(
          msg.gui, "time-out", "value", msg.value.getString());
      break;
    case Hash::calculateHash("url-in"):
      result = EBMessageOutSetProp(
          msg.gui, "url-out", "innerHTML", msg.value.getString());
      break;
    default:
      return ResultCode_t::UNKNOWN_HASH;
  }
  if (!result)
    return result;

  result = EBMessageOutEnqueue(msg.gui);
  if (!result)
    return result;
  return ResultCode_t::SUCCESS;
}

/**
 * @brief Process incoming message from the GUI
 *
 * @param msg to process
 * @return ResultCode_t error code
 */
ResultCode_t __stdcall guiProcess(const EBMessage_t & msg) {
  Result result;
  switch (msg.type) {
    case EBMSGType_t::STARTUP:
      EBLogInfo("Server starting up");
      break;
    case EBMSGType_t::SHUTDOWN:
      EBLogInfo("Server shutting down");
      break;
    case EBMSGType_t::INPUT:
      result = handleInput(msg);
      if (!result)
        EBLogError(result.getMessage());
      break;
    default:
      return EBDefaultGUIProcess(msg);
  }
  return ResultCode_t::SUCCESS;
}

int WINAPI WinMain(HINSTANCE, HINSTANCE, LPSTR, int) {
  ResultCode_t result =
      EBConfigureLogging("ehbanana.log", true, true, EB_LOG_LEVEL_DEBUG);
  if (!result)
    return static_cast<int>(result);

  EBLogInfo("Ehbanana test starting");

  EBGUISettings_t settings;
  settings.guiProcess = guiProcess;
  settings.configRoot = "test/config";
  settings.httpRoot   = "test/http";

  EBGUI_t gui = nullptr;
  result      = EBCreateGUI(settings, gui);
  if (!result) {
    EBLogError(EBGetLastResultMessage());
    return static_cast<int>(result);
  }

  result = EBShowGUI(gui);
  if (!result) {
    EBLogError(EBGetLastResultMessage());
    return static_cast<int>(result);
  }

  EBMessage_t msg;
  auto        nextUpdate = std::chrono::steady_clock::now();
  while ((result = EBGetMessage(msg)) == ResultCode_t::INCOMPLETE ||
         result == ResultCode_t::NO_OPERATION) {
    // If no messages were processed, wait a bit to save CPU
    if (result == ResultCode_t::NO_OPERATION) {
      std::this_thread::sleep_for(std::chrono::milliseconds(1));
    } else {
      result = EBDispatchMessage(msg);
      if (!result) {
        EBLogError(EBGetLastResultMessage());
        return static_cast<int>(result);
      }
    }
    if (std::chrono::steady_clock::now() > nextUpdate) {
      nextUpdate += std::chrono::milliseconds(500);
      result = EBMessageOutCreate(gui);
      if (!result) {
        EBLogError(EBGetLastResultMessage());
        return static_cast<int>(result);
      }

      result = EBMessageOutSetHref(gui, "/");
      if (!result) {
        EBLogError(EBGetLastResultMessage());
        return static_cast<int>(result);
      }

      result = EBMessageOutSetProp(
          gui, "stream-out", "innerHTML", std::to_string(rand() & 0xFF));
      if (!result) {
        EBLogError(EBGetLastResultMessage());
        return static_cast<int>(result);
      }

      result = EBMessageOutEnqueue(gui);
      if (!result) {
        EBLogError(EBGetLastResultMessage());
        return static_cast<int>(result);
      }
    }
  }

  if (!result) {
    EBLogError(EBGetLastResultMessage());
    return static_cast<int>(result);
  }

  result = EBDestroyGUI(gui);
  if (!result) {
    EBLogError(EBGetLastResultMessage());
    return static_cast<int>(result);
  }

  EBLogInfo("Ehbanana test complete");
  return static_cast<int>(ResultCode_t::SUCCESS);
}