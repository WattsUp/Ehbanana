#include <Ehbanana.h>

#include <chrono>
#include <thread>

/**
 * @brief Process incoming message from the GUI
 *
 * @param msg to process
 * @return EBResult_t error code
 */
EBResult_t __stdcall guiProcess(const EBMessage_t & msg) {
  switch (msg.type) {
    case EBMSGType_t::STARTUP:
      EBLogInfo("Server starting up");
      break;
    case EBMSGType_t::SHUTDOWN:
      EBLogInfo("Server shutting down");
      break;
    case EBMSGType_t::INPUT_FORM:
      if (msg.htmlID.empty())
        return EBRESULT_INVALID_DATA;
      EBLogInfo(("Received input from #" + msg.htmlID).c_str());
      if (msg.htmlID.compare("Exit")) {
        return EBEnqueueQuitMessage(msg.gui);
      }
      break;
    default:
      return EBDefaultGUIProcess(msg);
  }
  return EBRESULT_SUCCESS;
}

int WINAPI WinMain(
    HINSTANCE hInst, HINSTANCE hPrevInst, LPSTR cmdArgs, int cmdShow) {
  if (EBRESULT_ERROR(
          EBConfigureLogging("ehbanana.log", true, true, EB_LOG_LEVEL_DEBUG)))
    return EBGetLastResult();

  EBLogInfo("Ehbanana test starting");

  EBGUISettings_t settings;
  settings.guiProcess = guiProcess;
  settings.configRoot = "test/config";
  settings.httpRoot   = "test/http";

  EBGUI_t gui = EBCreateGUI(settings);
  if (gui == nullptr) {
    EBLogError(EBGetLastResultMessage());
    return EBGetLastResult();
  }

  if (EBRESULT_ERROR(EBShowGUI(gui))) {
    EBLogError(EBGetLastResultMessage());
    return EBGetLastResult();
  }

  EBMessage_t                           msg    = {};
  EBResult_t                            result = EBRESULT_SUCCESS;
  std::chrono::system_clock::time_point timeout =
      std::chrono::system_clock::now() + std::chrono::seconds(10);
  while (EBGetMessage(msg) == EBRESULT_INCOMPLETE_OPERATION) {
    result = EBDispatchMessage(msg);

    // If no messages were processed, wait a bit to save CPU
    if (result == EBRESULT_NO_OPERATION) {
      std::this_thread::sleep_for(std::chrono::milliseconds(1));

      // If no operations have occured for 10 seconds, stop
      if (std::chrono::system_clock::now() >= timeout) {
        if (EBRESULT_ERROR(EBEnqueueQuitMessage(gui))) {
          EBLogError(EBGetLastResultMessage());
          return EBGetLastResult();
        }
      }
    } else if (EBRESULT_ERROR(result)) {
      EBLogError(EBGetLastResultMessage());
      return EBGetLastResult();
    } else
      timeout = std::chrono::system_clock::now() + std::chrono::seconds(10);
  }

  if (EBRESULT_ERROR(EBGetLastResult())) {
    EBLogError(EBGetLastResultMessage());
    return EBGetLastResult();
  }

  if (EBRESULT_ERROR(EBDestroyGUI(gui))) {
    EBLogError(EBGetLastResultMessage());
    return EBGetLastResult();
  }

  EBLogInfo("Ehbanana test complete");
  return EBRESULT_SUCCESS;
}