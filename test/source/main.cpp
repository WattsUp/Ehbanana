#include <iostream>

#include <Ehbanana.h>

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
      std::cout << "Received input from " << msg.htmlID << "\n";
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

  if (EBRESULT_ERROR(EBEnqueueQuitMessage(gui)))
    return EBGetLastResult();

  EBMessage_t msg = {};
  while (EBGetMessage(msg) == EBRESULT_INCOMPLETE_OPERATION) {
    if (EBRESULT_ERROR(EBDispatchMessage(msg)))
      return EBGetLastResult();
  }

  if (EBRESULT_ERROR(EBGetLastResult()))
    return EBGetLastResult();

  return EBRESULT_SUCCESS;
}