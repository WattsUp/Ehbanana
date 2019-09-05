#include "Ehbanana.h"

#include "MessageOut.h"
#include "web/Server.h"

#include <FruitBowl.h>
#include <iostream>
#include <list>
//#include <spdlog/sinks/basic_file_sink.h>
//#include <spdlog/sinks/rotating_file_sink.h>
//#include <spdlog/sinks/stdout_sinks.h>
//#include <spdlog/spdlog.h>
#include <stdlib.h>
#include <string>

static std::list<EBMessage_t> EBMessageQueue;
static Result                 lastResult;

/**
 * @brief Create a process from the command string
 *
 * @param command string to execute
 * @return Result error code
 */

Result EBCreateProcess(
    const std::string & command, PROCESS_INFORMATION * process) {
  STARTUPINFOA startupInfo;
  ZeroMemory(&startupInfo, sizeof(startupInfo));
  startupInfo.cb = sizeof(startupInfo);
  char * buf     = new char[command.size() + 1];
  strcpy_s(buf, command.size() + 1, command.c_str());
  if (!CreateProcessA(
          NULL, buf, NULL, NULL, FALSE, 0, NULL, NULL, &startupInfo, process)) {
    // spdlog::error("Failed to create process with command \"{}\"", command);
    delete buf;
    return ResultCode_t::BAD_COMMAND + ("Create process: " + command);
  }
  delete buf;
  return ResultCode_t::SUCCESS;
}

const char * EBGetLastResultMessage() {
  return lastResult.getMessage();
}

/**
 * @brief Set the Last Result
 *
 * @param result to set
 * @return ResultCode_t of that result
 */
ResultCode_t setLastResult(Result result) {
  lastResult = result;
  return lastResult.getCode();
}

ResultCode_t EBCreateGUI(EBGUISettings_t guiSettings, EBGUI_t & gui) {
  // Construct a new EBGUI object to store settings and the server
  Result result = EBDestroyGUI(gui);
  if (!result)
    return setLastResult(result + "Desroying GUI before creating a new one");
  gui = new EBGUI();

  if (guiSettings.guiProcess == nullptr) {
    delete gui;
    return setLastResult(
        ResultCode_t::INVALID_PARAMETER + "guiProcess is nullptr");
  }

  // Construct a new server and attach it to the EBGUI
  gui->server = new Web::Server(gui);
  result = gui->server->configure(guiSettings.httpRoot, guiSettings.configRoot);
  if (!result) {
    if (result == ResultCode_t::OPEN_FAILED) {
      // Most likely: the working directory is not correct
      // Try again with the one folder up
      // spdlog::info("Could not open http and config root, trying one folder
      // up");
      result = gui->server->configure("../" + std::string(guiSettings.httpRoot),
          "../" + std::string(guiSettings.configRoot));
      if (!result) // No hope
        return setLastResult(result + "Configuring new server");
    } else
      return setLastResult(result + "Configuring new server");
  }

  result = gui->server->initializeSocket("127.0.0.1", guiSettings.httpPort);
  if (!result) {
    delete gui->server;
    delete gui;
    return setLastResult(result + "Initializing server's socket");
  }

  gui->server->start();

  result = EBEnqueueMessage({gui, EBMSGType_t::STARTUP});
  if (!result) {
    delete gui->server;
    delete gui;
    return setLastResult(result + "Enqueueing STARTUP message");
  }

  gui->settings = guiSettings;
  return setLastResult(ResultCode_t::SUCCESS);
}

ResultCode_t EBShowGUI(EBGUI_t gui) {
  // Ensure system calls is available
  if (!std::system(nullptr))
    return setLastResult(ResultCode_t::NO_SYSTEM_CALL);

  PROCESS_INFORMATION browser;

  std::string URL = "http://";
  URL += gui->server->getDomainName();

  std::string command = "--app=\"" + URL + "\"";

  // Try Chrome default install location first
  command =
      "\"C:\\Program Files (x86)\\Google\\Chrome\\Application\\chrome.exe\"";
  command += " --app=\"" + URL + "\"";
  if (!EBCreateProcess(command, &browser)) {
    // spdlog::warn("Chrome not found at default installation");
    // Try app data next
    command = "\"%LocalAppData%\\Google\\Chrome\\Application\\chrome.exe\"";
    command += " --app=\"" + URL + "\"";
    if (!EBCreateProcess(command, &browser)) {
      // spdlog::warn("Chrome not found in app data");
      // Use default browser last
      command = "cmd /c start " + URL;
      if (!EBCreateProcess(command, &browser))
        return setLastResult(
            ResultCode_t::OPEN_FAILED + "Failed to start a web browser");
    }
  }

  // Wait for the process to finish spawning the browser up to 1s
  WaitForSingleObject(browser.hProcess, 1000);
  CloseHandle(browser.hProcess);
  CloseHandle(browser.hThread);

  // spdlog::info("Web browser opened to {}", URL);

  return setLastResult(ResultCode_t::SUCCESS);
}

ResultCode_t EBDestroyGUI(EBGUI_t gui) {
  if (gui != nullptr)
    delete gui->server;
  delete gui;
  return setLastResult(ResultCode_t::SUCCESS);
}

ResultCode_t EBGetMessage(EBMessage_t & msg) {
  if (EBMessageQueue.empty()) {
    msg.type = EBMSGType_t::NONE;
    return setLastResult(ResultCode_t::NO_OPERATION);
  }
  msg = EBMessageQueue.front();
  EBMessageQueue.pop_front();
  if (msg.type == EBMSGType_t::QUIT) {
    msg.gui->server->stop();
    return setLastResult(ResultCode_t::SUCCESS);
  }
  return setLastResult(ResultCode_t::INCOMPLETE);
}

ResultCode_t EBDispatchMessage(const EBMessage_t & msg) {
  Result result = (msg.gui->settings.guiProcess)(msg);
  return setLastResult(result + "Dispatched message to guiProcess");
}

ResultCode_t EBEnqueueMessage(const EBMessage_t & msg) {
  EBMessageQueue.push_back(msg);
  return setLastResult(ResultCode_t::SUCCESS);
}

ResultCode_t EBDefaultGUIProcess(const EBMessage_t & msg) {
  return setLastResult(ResultCode_t::NOT_SUPPORTED + "Default GUI process");
}

ResultCode_t EBMessageOutCreate(EBGUI_t gui) {
  if (gui->currentMessageOut == nullptr)
    delete gui->currentMessageOut;

  gui->currentMessageOut = new MessageOut();
  return setLastResult(ResultCode_t::SUCCESS);
}

ResultCode_t EBMessageOutSetHref(EBGUI_t gui, const char * href) {
  if (gui->currentMessageOut == nullptr)
    return setLastResult(
        ResultCode_t::INVALID_DATA + "currentMessageOut is nullptr");
  Result result = gui->currentMessageOut->setHref(href);
  return setLastResult(result + "Setting message out href");
}

ResultCode_t EBMessageOutSetProp(
    EBGUI_t gui, const char * id, const char * name, const char * value) {
  if (gui->currentMessageOut == nullptr)
    return setLastResult(
        ResultCode_t::INVALID_DATA + "currentMessageOut is nullptr");
  Result result = gui->currentMessageOut->setProperty(id, name, value);
  return setLastResult(result + "Setting message out property");
}

ResultCode_t EBMessageOutEnqueue(EBGUI_t gui) {
  if (gui->currentMessageOut == nullptr)
    return setLastResult(
        ResultCode_t::INVALID_DATA + "currentMessageOut is nullptr");
  if (!gui->currentMessageOut->isEnqueued())
    gui->server->enqueueOutput(gui->currentMessageOut->getString());
  return setLastResult(ResultCode_t::SUCCESS);
}