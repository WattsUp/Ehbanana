#include <Ehbanana.h>

#include <chrono>
#include <iostream>
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
      std::cout << "Server starting up\n";
      break;
    case EBMSGType_t::SHUTDOWN:
      std::cout << "Server shutting down\n";
      break;
    case EBMSGType_t::INPUT_FORM:
      if (msg.htmlID == nullptr)
        return EBRESULT_INVALID_DATA;
      if (strcmp(msg.htmlID, "Exit") == 0) {
        if (EBRESULT_ERROR(EBEnqueueQuitMessage(msg.gui)))
          return EBGetLastResult();
      }
      std::cout << "Received input from #" << msg.htmlID << "\n";
      break;
    default:
      return EBDefaultGUIProcess(msg);
  }
  return EBRESULT_SUCCESS;
}

int main() {
  std::cout << "Ehbanana test\n";
  EBGUISettings_t settings;
  settings.guiProcess = guiProcess;
  settings.configRoot = "test/config";
  settings.httpRoot   = "test/http";

  EBGUI_t gui = EBCreateGUI(settings);
  if (gui == nullptr)
    return EBGetLastResult();

  if (EBRESULT_ERROR(EBShowGUI(gui)))
    return EBGetLastResult();

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
        if (EBRESULT_ERROR(EBEnqueueQuitMessage(gui)))
          return EBGetLastResult();
      }
    } else if (EBRESULT_ERROR(result))
      return result;
    else
      timeout = std::chrono::system_clock::now() + std::chrono::seconds(10);
  }

  if (EBRESULT_ERROR(EBGetLastResult()))
    return EBGetLastResult();

  if (EBRESULT_ERROR(EBDestroyGUI(gui)))
    return EBGetLastResult();

  return EBRESULT_SUCCESS;
}