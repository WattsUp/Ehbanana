#include <Ehbanana.h>

#include <thread>

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
    case EBMSGType_t::INPUT_FORM:
      if (msg.htmlID.empty())
        return ResultCode_t::INVALID_DATA;
      EBLogInfo(("Received input from #" + msg.htmlID).c_str());
      if (msg.htmlID.compare("Exit")) {
        return EBEnqueueQuitMessage(msg.gui);
      }
      break;
    default:
      return EBDefaultGUIProcess(msg);
  }
  return ResultCode_t::SUCCESS;
}

int WINAPI WinMain(
    HINSTANCE hInst, HINSTANCE hPrevInst, LPSTR cmdArgs, int cmdShow) {
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