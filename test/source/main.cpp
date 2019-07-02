#include <iostream>

#include <Ehbanana.h>

/**
 * @brief Process incoming message from the GUI
 * 
 * @param msg to process
 * @return EBResult_t error code 
 */
EBResult_t __stdcall guiProcess(EBMessage_t msg) {
  std::cout << msg.htmlID << " " << (uint16_t)msg.type << "\n";
  return EBRESULT_NOT_SUPPORTED;
}

int main() {
  std::cout << "Ehbanana test\n";
  EBGUISettings_t settings;
  settings.guiProcess = guiProcess;
  settings.configRoot = "test/config";
  settings.httpRoot   = "test/http";

  EBGUI_t gui = EBCreateGUI(settings);
  if (gui == nullptr) {
    return EBGetLastResult();
  }

  if (EBRESULT_ERROR(EBShowGUI(gui))) {
    return EBGetLastResult();
  }
}