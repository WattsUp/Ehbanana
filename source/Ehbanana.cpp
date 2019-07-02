#include "Ehbanana.h"
#include <iostream>
#include <list>
#include <stdlib.h>
#include <string>

static EBResult_t             EBLastError = EBRESULT_SUCCESS;
static std::list<EBMessage_t> EBMessageQueue;

EBGUI_t EBCreateGUI(EBGUISettings_t guiSettings) {
  EBGUI_t gui = new EBGUI();
  if (guiSettings.port == EBPORT_AUTO)
    guiSettings.port = EBPORT_DEFAULT;
  gui->settings = guiSettings;
  if (EBEnqueueMessage({gui, EBMSGType_t::STARTUP})) {
    delete gui;
    return nullptr;
  }
  return gui;
}

EBResult_t EBShowGUI(EBGUI_t gui) {
  // Ensure system calls is available
  if (!std::system(nullptr))
    return EBSetLastResult(EBRESULT_OPEN_FAILED);

  PROCESS_INFORMATION browser;

  std::string URL = "http://127.0.0.1:" + std::to_string(gui->settings.port);
  std::string command = "--app=\"" + URL + "\"";

  // Try Chrome default install location first
  command =
      "\"C:\\Program Files (x86)\\Google\\Chrome\\Application\\chrome.exe\"";
  command += " --app=\"" + URL + "\"";
  if (!EBCreateProcess(&command[0], &browser) == EBRESULT_SUCCESS) {
    // Try app data next
    command = "\"%LocalAppData%\\Google\\Chrome\\Application\\chrome.exe\"";
    command += " --app=\"" + URL + "\"";
    if (!EBCreateProcess(&command[0], &browser) == EBRESULT_SUCCESS) {
      // Use default browser instead
      command = "cmd /c start " + URL;
      if (!EBCreateProcess(&command[0], &browser) == EBRESULT_SUCCESS)
        return EBSetLastResult(EBRESULT_OPEN_FAILED);
    }
  }

  // Wait for the process to finish spawning the browser up to 1s
  WaitForSingleObject(browser.hProcess, 1000);
  CloseHandle(browser.hProcess);
  CloseHandle(browser.hThread);

  return EBSetLastResult(EBRESULT_SUCCESS);
}

EBResult_t EBGetLastResult() {
  return EBLastError;
}

EBResult_t EBSetLastResult(EBResult_t result) {
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

EBResult_t EBCreateProcess(char * command, PROCESS_INFORMATION * process) {
  STARTUPINFOA startupInfo;
  ZeroMemory(&startupInfo, sizeof(startupInfo));
  startupInfo.cb = sizeof(startupInfo);
  if (!CreateProcessA(NULL, command, NULL, NULL, FALSE, 0, NULL, NULL,
          &startupInfo, process))
    return EBSetLastResult(EBRESULT_BAD_COMMAND);
  return EBSetLastResult(EBRESULT_SUCCESS);
}