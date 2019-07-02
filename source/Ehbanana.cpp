#include "Ehbanana.h"
#include <iostream>

static EBResult_t EBLastError = EBRESULT_SUCCESS;

EBGUI_t EBCreateGUI(EBGUISettings_t guiSettings) {
  std::cout << "It's ehbanana DLL\n";
  EBGUI_t gui   = new EBGUI();
  gui->settings = guiSettings;
  (gui->settings.guiProcess)({EBMSGType_t::INPUT_FORM, "Hello"});
  return gui;
}

EBResult_t EBShowGUI(EBGUI_t gui) {
  return EBSetLastResult(EBRESULT_NOT_SUPPORTED);
}

EBResult_t EBGetLastResult() {
  return EBLastError;
}

EBResult_t EBSetLastResult(EBResult_t result) {
  EBLastError = result;
  return EBLastError;
}