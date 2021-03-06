#include "GUI.h"

#include <thread>

namespace GUI {

/**
 * @brief Destroy the GUI::GUI object
 *
 */
GUI::~GUI() {
  delete root;
}

/**
 * @brief Process incoming message from the GUI
 *
 * @param msg to process
 * @return ResultCode_t error code
 */
ResultCode_t __stdcall GUI::guiProcess(const EBMessage_t & msg) {
  switch (msg.type) {
    case EBMSGType_t::STARTUP:
      printf("Server starting up");
      break;
    case EBMSGType_t::SHUTDOWN:
      printf("Server shutting down");
      break;
    case EBMSGType_t::INPUT: {
      Result result = GUI::GUI::Instance()->handleInput(msg);
      if (!result)
        printf(result.getMessage());
      return result.getCode();
    }
    default:
      return EBDefaultGUIProcess(msg);
  }
  return ResultCode_t::SUCCESS;
}

/**
 * @brief Initialize the GUI by creating an Ehbanana instance
 *
 * @return Result
 */
Result GUI::init() {
  EBGUISettings_t settings;
  settings.guiProcess = guiProcess;
  settings.configRoot = "test/config";
  settings.httpRoot   = "test/http";

  ResultCode_t resultCode = EBCreateGUI(settings, gui);
  if (!resultCode)
    return resultCode + "EBCreateGUI";

  root = new Root(gui);

  resultCode = EBShowGUI(gui);
  if (!resultCode)
    return resultCode + "EBShowGUI";

  return ResultCode_t::SUCCESS;
}

/**
 * @brief Run a loop checking for messages from the GUI, will block until GUI is
 * complete
 *
 * @return Result
 */
Result GUI::run() {
  Result      result;
  EBMessage_t msg;
  auto        nextPeriodic = clockStd_t::now();
  auto        now          = clockStd_t::now();
  while ((result = EBGetMessage(msg)) == ResultCode_t::INCOMPLETE ||
         result == ResultCode_t::NO_OPERATION) {
    // If no messages were processed, wait a bit to save CPU
    if (result == ResultCode_t::NO_OPERATION) {
      std::this_thread::sleep_for(std::chrono::milliseconds(1));
    } else {
      result = EBDispatchMessage(msg);
      if (!result)
        return result + "EBDispatchMessage";
    }

    now = clockStd_t::now();
    if (now > nextPeriodic) {
      nextPeriodic += std::chrono::milliseconds(500);
      result = root->sendUpdate();
      if (!result)
        return result + "Sending periodic update";
    }
  }
  return result + "EBGetMessage";
}

/**
 * @brief Deinitialize the GUI
 *
 * @return Result
 */
Result GUI::deinit() {
  Result result = EBDestroyGUI(gui);
  if (!result)
    return result + "EBDestroyGUI";
  return ResultCode_t::SUCCESS;
}

/**
 * @brief Handle the input from the GUI
 *
 * @param msg to handle
 * @return Result
 */
Result GUI::handleInput(const EBMessage_t & msg) {
  if (msg.id.getString().empty())
    return ResultCode_t::INVALID_DATA;
  printf("%s|%s|%s\n", msg.href.getString().c_str(), msg.id.getString().c_str(),
      msg.value.getString().c_str());

  switch (msg.href.get()) {
    case Hash::calculateHash(""):
    case Hash::calculateHash("/"):
      return root->handleInput(msg);
    default:
      return ResultCode_t::UNKNOWN_HASH +
             ("Input message's href: " + msg.href.getString());
  }
  return ResultCode_t::SUCCESS;
}

} // namespace GUI