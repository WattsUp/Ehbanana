#include "Ehbanana.h"

#include "web/Server.h"

#include <iostream>
#include <list>
#include <stdlib.h>

static EBResult_t             EBLastError = EBRESULT_SUCCESS;
static std::list<EBMessage_t> EBMessageQueue;

EBGUI_t EBCreateGUI(EBGUISettings_t guiSettings) {
  // Construct a new EBGUI object to store settings and the server
  EBGUI_t gui   = new EBGUI();
  gui->settings = guiSettings;

  // Construct a new server and attach it to the EBGUI
  gui->server = new Web::Server(guiSettings.httpRoot, guiSettings.configRoot);

  if (EBRESULT_ERROR(EBSetLastResult(gui->server->initialize("127.0.0.1")))) {
    delete gui->server;
    delete gui;
    return nullptr;
  }

  if (EBRESULT_ERROR(EBSetLastResult(gui->server->start()))) {
    delete gui->server;
    delete gui;
    return nullptr;
  }

  if (EBEnqueueMessage({gui, EBMSGType_t::STARTUP})) {
    delete gui;
    return nullptr;
  }
  return gui;
}

EBResult_t EBShowGUI(EBGUI_t gui) {
  // Ensure system calls is available
  if (!std::system(nullptr))
    return EBSetLastResult(EBRESULT_NO_SYSTEM_CALL);

  PROCESS_INFORMATION browser;

  std::string URL = "http://";
  URL += gui->server->getDomainName();

  std::string command = "--app=\"" + URL + "\"";

  // Try Chrome default install location first
  command =
      "\"C:\\Program Files (x86)\\Google\\Chrome\\Application\\chrome.exe\"";
  command += " --app=\"" + URL + "\"";
  if (!EBCreateProcess(command, &browser) == EBRESULT_SUCCESS) {
    // Try app data next
    command = "\"%LocalAppData%\\Google\\Chrome\\Application\\chrome.exe\"";
    command += " --app=\"" + URL + "\"";
    if (!EBCreateProcess(command, &browser) == EBRESULT_SUCCESS) {
      // Use default browser last
      command = "cmd /c start " + URL;
      if (!EBCreateProcess(command, &browser) == EBRESULT_SUCCESS)
        return EBSetLastResult(EBRESULT_OPEN_FAILED);
    }
  }

  // Wait for the process to finish spawning the browser up to 1s
  WaitForSingleObject(browser.hProcess, 1000);
  CloseHandle(browser.hProcess);
  CloseHandle(browser.hThread);

  return EBSetLastResult(EBRESULT_SUCCESS);
}

EBResult_t EBDestroyGUI(EBGUI_t gui) {
  delete gui->server;
  delete gui;
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
  if (msg.type == EBMSGType_t::QUIT) {
    if (EBRESULT_ERROR(EBSetLastResult(msg.gui->server->stop()))) {
      return EBGetLastResult();
    }
    return EBSetLastResult(EBRESULT_SUCCESS);
  }
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

EBResult_t EBCreateProcess(
    const std::string & command, PROCESS_INFORMATION * process) {
  STARTUPINFOA startupInfo;
  ZeroMemory(&startupInfo, sizeof(startupInfo));
  startupInfo.cb = sizeof(startupInfo);
  char * buf     = new char[command.size() + 1];
  strcpy_s(buf, command.size() + 1, command.c_str());
  if (!CreateProcessA(
          NULL, buf, NULL, NULL, FALSE, 0, NULL, NULL, &startupInfo, process)) {
    delete buf;
    return EBSetLastResult(EBRESULT_BAD_COMMAND);
  }
  delete buf;
  return EBSetLastResult(EBRESULT_SUCCESS);
}