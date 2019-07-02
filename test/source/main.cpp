#include <Ehbanana.h>

#include <iostream>
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

  if (EBRESULT_ERROR(EBEnqueueMessage({gui, EBMSGType_t::INPUT_FORM, "Exit"})))
    return EBGetLastResult();

  EBMessage_t msg = {};
  EBResult_t result = EBRESULT_SUCCESS;
  while (EBGetMessage(msg) == EBRESULT_INCOMPLETE_OPERATION) {
    result = EBDispatchMessage(msg);
    // If no messages were processed, wait a bit to save CPU
    if(result == EBRESULT_NO_OPERATION)
      std::this_thread::sleep_for(std::chrono::microseconds(100));
    else if (EBRESULT_ERROR(result))
      return result;
  }

  if (EBRESULT_ERROR(EBGetLastResult()))
    return EBGetLastResult();

  return EBRESULT_SUCCESS;
}