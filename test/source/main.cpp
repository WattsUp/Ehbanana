#include <Ehbanana.h>
#include <algorithm/sha1.hpp>
#include <base64.h>
#include <chrono>
#include <rapidjson/document.h>
#include <rapidjson/prettywriter.h>
#include <thread>

bool appleSelected  = false;
bool bananaSelected = false;
bool orangeSelected = false;

/**
 * @brief Handle an input message
 *
 * @param msg
 * @return ResultCode_t
 */
ResultCode_t handleInput(const EBMessage_t & msg) {
  rapidjson::StringBuffer                          sb;
  rapidjson::PrettyWriter<rapidjson::StringBuffer> writer(sb);
  msg.body->Accept(writer);
  EBLogInfo(sb.GetString());
  // if (msg.htmlID.getString().empty())
  //   return ResultCode_t::INVALID_DATA;
  // EBLogInfo(("Received input from #" + msg.htmlID.getString() + " value: \""
  // +
  //            msg.htmlValue.getString() + "\"")
  //               .c_str());

  EBMessage_t msgOut;
  msgOut.gui  = msg.gui;
  msgOut.type = EBMSGType_t::OUTPUT;
  // switch (msg.htmlID.get()) {
  //   case Hash::calculateHash("text-in"): {
  //     std::string temp(msg.htmlValue.getString());
  //     std::reverse(temp.begin(), temp.end());
  //     msgOut.htmlID.add("text-out");
  //     msgOut.htmlValue.add(temp);
  //   } break;
  //   case Hash::calculateHash("password-in"): {
  //     digestpp::sha1 sha;
  //     sha.absorb(msg.htmlValue.getString());
  //     uint8_t buf[20];
  //     sha.digest(buf, 20);
  //     msgOut.htmlID.add("password-out");
  //     msgOut.htmlValue.add(base64_encode(buf, 20));
  //   } break;
  //   case Hash::calculateHash("life-in"):
  //     msgOut.htmlID.add("life-out");
  //     msgOut.htmlValue.add("42");
  //     break;
  //   case Hash::calculateHash("fruit"):
  //     msgOut.htmlID.add("fruit-out");
  //     if (msg.htmlValue.get() == Hash::calculateHash("Banana"))
  //       msgOut.htmlValue.add("You have chosen <i>wisely</i>");
  //     else if (msg.htmlValue.get() == Hash::calculateHash("Apple"))
  //       msgOut.htmlValue.add("You have chosen <i>wrong</i>");
  //     else
  //       msgOut.htmlValue.add("You have chosen <i>poorly</i>");
  //     break;
  //   case Hash::calculateHash("fruit-checkbox0"):
  //     msgOut.htmlID.add("fruit-check-out");
  //     msgOut.htmlValue.add("I have a ");
  //     appleSelected = msg.checked.get() == Hash::calculateHash("true");
  //     if (appleSelected)
  //       msgOut.htmlValue.add("apple ");
  //     if (bananaSelected)
  //       msgOut.htmlValue.add("banana ");
  //     if (orangeSelected)
  //       msgOut.htmlValue.add("orange ");
  //     break;
  //   case Hash::calculateHash("fruit-checkbox1"):
  //     msgOut.htmlID.add("fruit-check-out");
  //     msgOut.htmlValue.add("I have a ");
  //     bananaSelected = msg.checked.get() == Hash::calculateHash("true");
  //     if (appleSelected)
  //       msgOut.htmlValue.add("apple ");
  //     if (bananaSelected)
  //       msgOut.htmlValue.add("banana ");
  //     if (orangeSelected)
  //       msgOut.htmlValue.add("orange ");
  //     break;
  //   case Hash::calculateHash("fruit-checkbox2"):
  //     msgOut.htmlID.add("fruit-check-out");
  //     msgOut.htmlValue.add("I have a ");
  //     orangeSelected = msg.checked.get() == Hash::calculateHash("true");
  //     if (appleSelected)
  //       msgOut.htmlValue.add("apple ");
  //     if (bananaSelected)
  //       msgOut.htmlValue.add("banana ");
  //     if (orangeSelected)
  //       msgOut.htmlValue.add("orange ");
  //     break;
  //   case Hash::calculateHash("color-in"):
  //     msgOut.htmlID.add("color-out");
  //     msgOut.htmlValue.add(msg.htmlValue.getString());
  //     break;
  //   case Hash::calculateHash("date-in"):
  //     msgOut.htmlID.add("date-out");
  //     msgOut.htmlValue.add(msg.htmlValue.getString());
  //     break;
  //   case Hash::calculateHash("datetime-in"):
  //     msgOut.htmlID.add("datetime-out");
  //     msgOut.htmlValue.add(msg.htmlValue.getString());
  //     break;
  //   case Hash::calculateHash("email-in"):
  //     msgOut.htmlID.add("email-out");
  //     msgOut.htmlValue.add(msg.htmlValue.getString());
  //     break;
  //   case Hash::calculateHash("file-in"): {
  //     msgOut.htmlID.add("file-out");
  //     if (msg.file == nullptr)
  //       return ResultCode_t::INVALID_DATA;
  //     digestpp::sha1 sha;
  //     int            i;
  //     while ((i = fgetc(msg.file)) != EOF){
  //       char c = static_cast<char>(i);
  //       sha.absorb(&c, 1);
  //     }
  //     fclose(msg.file);
  //     uint8_t shaBuf[20];
  //     sha.digest(shaBuf, 20);
  //     msgOut.htmlValue.add(base64_encode(shaBuf, 20));
  //   } break;
  //   case Hash::calculateHash("month-in"):
  //     msgOut.htmlID.add("month-out");
  //     msgOut.htmlValue.add(msg.htmlValue.getString());
  //     break;
  //   case Hash::calculateHash("number-in"):
  //     msgOut.htmlID.add("number-out");
  //     msgOut.htmlValue.add(msg.htmlValue.getString());
  //     break;
  //   case Hash::calculateHash("range-in"):
  //     msgOut.htmlID.add("range-out");
  //     msgOut.htmlValue.add(msg.htmlValue.getString());
  //     break;
  //   case Hash::calculateHash("tel-in"):
  //     msgOut.htmlID.add("tel-out");
  //     msgOut.htmlValue.add(msg.htmlValue.getString());
  //     break;
  //   case Hash::calculateHash("time-in"):
  //     msgOut.htmlID.add("time-out");
  //     msgOut.htmlValue.add(msg.htmlValue.getString());
  //     break;
  //   case Hash::calculateHash("url-in"):
  //     msgOut.htmlID.add("url-out");
  //     msgOut.htmlValue.add(msg.htmlValue.getString());
  //     break;
  //   default:
  //     return ResultCode_t::UNKNOWN_HASH;
  // }
  EBEnqueueMessage(msgOut);
  return ResultCode_t::SUCCESS;
}

/**
 * @brief Process incoming message from the GUI
 *
 * @param msg to process
 * @return ResultCode_t error code
 */
ResultCode_t __stdcall guiProcess(const EBMessage_t & msg) {
  switch (msg.type) {
    case EBMSGType_t::STARTUP:
      EBLogInfo("Server starting up");
      break;
    case EBMSGType_t::SHUTDOWN:
      EBLogInfo("Server shutting down");
      break;
    case EBMSGType_t::INPUT:
      return handleInput(msg);
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
    if (std::chrono::steady_clock::now() > nextUpdate) {
      nextUpdate += std::chrono::milliseconds(500);
      EBMessage_t streamUpdate;
      streamUpdate.gui  = gui;
      streamUpdate.type = EBMSGType_t::OUTPUT;
      streamUpdate.body = std::make_shared<rapidjson::Document>();
      streamUpdate.body->SetObject();
      streamUpdate.body->AddMember("href", rapidjson::StringRef(""), streamUpdate.body->GetAllocator());
      rapidjson::Value elements;
      elements.SetObject();
      rapidjson::Value streamOut;
      streamOut.SetObject();
      streamOut.AddMember("innerHTML", rapidjson::StringRef(std::to_string(rand() & 0xFF).c_str()), streamUpdate.body->GetAllocator());
      elements.AddMember("stream-out", streamOut, streamUpdate.body->GetAllocator());
      streamUpdate.body->AddMember("elements", elements, streamUpdate.body->GetAllocator());
      EBEnqueueMessage(streamUpdate);
    }
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