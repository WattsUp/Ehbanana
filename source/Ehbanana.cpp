#include "Ehbanana.h"
#include <iostream>
#include <list>

static EBResult_t             EBLastError = EBRESULT_SUCCESS;
static std::list<EBMessage_t> EBMessageQueue;

EBGUI_t EBCreateGUI(EBGUISettings_t guiSettings) {
  EBGUI_t gui   = new EBGUI();
  gui->settings = guiSettings;
  if (EBEnqueueMessage({gui, EBMSGType_t::STARTUP})) {
    delete gui;
    return nullptr;
  }
  return gui;
}

EBResult_t EBShowGUI(EBGUI_t gui) {
  return EBSetLastResult(EBRESULT_SUCCESS);
}

inline EBResult_t EBGetLastResult() {
  return EBLastError;
}

inline EBResult_t EBSetLastResult(EBResult_t result) {
  EBLastError = result;
  return EBLastError;
}

EBResult_t EBGetMessage(EBMessage_t & msg) {
  if (EBMessageQueue.empty()) {
    msg.type = EBMSGType_t::NONE;
    return EBSetLastResult(EBRESULT_INCOMPLETE_OPERATION);
  }
  msg = EBMessageQueue.front();
  EBMessageQueue.pop_front();
  if (msg.type == EBMSGType_t::QUIT)
    return EBSetLastResult(EBRESULT_SUCCESS);
  return EBSetLastResult(EBRESULT_INCOMPLETE_OPERATION);
}

EBResult_t EBDispatchMessage(const EBMessage_t & msg) {
  if (msg.type == EBMSGType_t::NONE)
    return EBSetLastResult(EBRESULT_NO_OPERATION);
  return EBSetLastResult((msg.gui->settings.guiProcess)(msg));
}

EBResult_t EBEnqueueMessage(const EBMessage_t & msg) {
  EBMessageQueue.push_back(msg);
  return EBSetLastResult(EBRESULT_SUCCESS);
}

EBResult_t EBDefaultGUIProcess(const EBMessage_t & msg) {
  std::cout << "GUI sent message of type " << (uint16_t)msg.type << "\n";
  return EBSetLastResult(EBRESULT_NOT_SUPPORTED);
}