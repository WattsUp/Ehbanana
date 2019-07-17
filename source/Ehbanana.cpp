#include "Ehbanana.h"

#include "ResultMsg.h"
#include "web/Server.h"

#include <iostream>
#include <list>
#include <spdlog/sinks/basic_file_sink.h>
#include <spdlog/sinks/rotating_file_sink.h>
#include <spdlog/sinks/stdout_sinks.h>
#include <spdlog/spdlog.h>
#include <stdlib.h>
#include <string>

static EBResultMsg_t          EBLastError = EBResult::SUCCESS;
static std::list<EBMessage_t> EBMessageQueue;

/**
 * @brief Create a process from the command string
 *
 * @param command string to execute
 * @return EBResultMsg_t error code
 */

EBResultMsg_t EBCreateProcess(
    const std::string & command, PROCESS_INFORMATION * process) {
  STARTUPINFOA startupInfo;
  ZeroMemory(&startupInfo, sizeof(startupInfo));
  startupInfo.cb = sizeof(startupInfo);
  char * buf     = new char[command.size() + 1];
  strcpy_s(buf, command.size() + 1, command.c_str());
  if (!CreateProcessA(
          NULL, buf, NULL, NULL, FALSE, 0, NULL, NULL, &startupInfo, process)) {
    spdlog::error("Failed to create process with command \"{}\"", command);
    delete buf;
    return EBResult::BAD_COMMAND + ("Create process: " + command);
  }
  delete buf;
  return EBResult::SUCCESS;
}

EBResult_t EBGetLastResult() {
  return EBLastError.value;
}

const char * EBGetLastResultMessage() {
  return EBLastError.message.c_str();
}

/**
 * @brief Set the last error produced by Ehbanana
 *
 * @param result to set
 * @return last result
 */
EBResult_t EBSetLastResult(EBResultMsg_t result) {
  EBLastError = result;
  return EBLastError.value;
}

EBGUI_t EBCreateGUI(EBGUISettings_t guiSettings) {
  // Construct a new EBGUI object to store settings and the server
  EBGUI_t gui   = new EBGUI();
  gui->settings = guiSettings;

  if (gui->settings.guiProcess == nullptr) {
    EBSetLastResult(EBResult::INVALID_PARAMETER + "guiProcess is nullptr");
    delete gui;
    return nullptr;
  }

  // Construct a new server and attach it to the EBGUI
  gui->server = new Web::Server(guiSettings.httpRoot, guiSettings.configRoot);

  EBResultMsg_t result = gui->server->initialize("127.0.0.1");
  if (EBRESULT_ERROR(EBSetLastResult(result))) {
    delete gui->server;
    delete gui;
    return nullptr;
  }

  result = gui->server->start();
  if (EBRESULT_ERROR(EBSetLastResult(result))) {
    delete gui->server;
    delete gui;
    return nullptr;
  }

  result = {EBEnqueueMessage({gui, EBMSGType_t::STARTUP}),
      "During enqueueing startup message"};
  if (EBRESULT_ERROR(EBSetLastResult(result))) {
    delete gui->server;
    delete gui;
    return nullptr;
  }
  return gui;
}

EBResult_t EBShowGUI(EBGUI_t gui) {
  // Ensure system calls is available
  if (!std::system(nullptr))
    return EBSetLastResult(EBResult::NO_SYSTEM_CALL);

  PROCESS_INFORMATION browser;

  std::string URL = "http://";
  URL += gui->server->getDomainName();

  std::string command = "--app=\"" + URL + "\"";

  // Try Chrome default install location first
  command =
      "\"C:\\Program Files (x86)\\Google\\Chrome\\Application\\chrome.exe\"";
  command += " --app=\"" + URL + "\"";
  if (!EBCreateProcess(command, &browser)) {
    spdlog::warn("Chrome not found at default installation");
    // Try app data next
    command = "\"%LocalAppData%\\Google\\Chrome\\Application\\chrome.exe\"";
    command += " --app=\"" + URL + "\"";
    if (!EBCreateProcess(command, &browser)) {
      spdlog::warn("Chrome not found in app data");
      // Use default browser last
      command = "cmd /c start " + URL;
      if (!EBCreateProcess(command, &browser))
        return EBSetLastResult(
            EBResult::OPEN_FAILED + "Failed to start a web browser");
    }
  }

  // Wait for the process to finish spawning the browser up to 1s
  WaitForSingleObject(browser.hProcess, 1000);
  CloseHandle(browser.hProcess);
  CloseHandle(browser.hThread);

  spdlog::info("Web browser opened to {}", URL);

  return EBSetLastResult(EBResult::SUCCESS);
}

EBResult_t EBDestroyGUI(EBGUI_t gui) {
  delete gui->server;
  delete gui;
  return EBSetLastResult(EBResult::SUCCESS);
}

EBResult_t EBGetMessage(EBMessage_t & msg) {
  if (EBMessageQueue.empty()) {
    msg.type = EBMSGType_t::NONE;
    return EBSetLastResult(EBResult::INCOMPLETE_OPERATION);
  }
  msg = EBMessageQueue.front();
  EBMessageQueue.pop_front();
  if (msg.type == EBMSGType_t::QUIT) {
    EBResultMsg_t result = msg.gui->server->stop();
    if (!result) {
      spdlog::error(result);
      return EBSetLastResult(result);
    }
    return EBSetLastResult(EBResult::SUCCESS);
  }
  return EBSetLastResult(EBResult::INCOMPLETE_OPERATION);
}

EBResult_t EBDispatchMessage(const EBMessage_t & msg) {
  if (msg.type == EBMSGType_t::NONE)
    return EBSetLastResult(EBResult::NO_OPERATION);
  return EBSetLastResult(
      {(msg.gui->settings.guiProcess)(msg), "Error during guiProcess"});
}

EBResult_t EBEnqueueMessage(const EBMessage_t & msg) {
  EBMessageQueue.push_back(msg);
  return EBSetLastResult(EBResult::SUCCESS);
}

EBResult_t EBDefaultGUIProcess(const EBMessage_t & msg) {
  std::cout << "GUI sent message of type " << (uint16_t)msg.type << "\n";
  return EBSetLastResult(EBResult::NOT_SUPPORTED + "DefaultGUIProcess");
}

EBResult_t EBConfigureLogging(const char * fileName, bool rotatingLogs,
    bool showConsole, uint8_t logLevel) {
  std::vector<spdlog::sink_ptr> sinks;

  if (showConsole) {
    // Create a console and remove its close button
    if (AllocConsole()) {
      HWND hwnd = GetConsoleWindow();
      if (hwnd != NULL) {
        HMENU hMenu = GetSystemMenu(hwnd, FALSE);
        if (hMenu != NULL)
          DeleteMenu(hMenu, SC_CLOSE, MF_BYCOMMAND);
      }
    } else {
      MessageBoxA(NULL, "Log console initialization failed", "Error", MB_OK);
      std::cout << "Failed to AllocConsole with Win32 error: " << GetLastError()
                << std::endl;
      return EBRESULT_OPEN_FAILED;
    }
    sinks.push_back(std::make_shared<spdlog::sinks::wincolor_stdout_sink_mt>());
  }

  if (fileName != nullptr) {
    try {
      if (rotatingLogs)
        sinks.push_back(std::make_shared<spdlog::sinks::rotating_file_sink_mt>(
            fileName, 5 * 1024 * 1024, 3));
      else
        sinks.push_back(
            std::make_shared<spdlog::sinks::basic_file_sink_mt>(fileName));
    } catch (const spdlog::spdlog_ex & e) {
      MessageBoxA(NULL, "Log initialization failed", "Error", MB_OK);
      std::cout << "Log initialization failed: " << e.what() << std::endl;
      return EBRESULT_OPEN_FAILED;
    }
  }

  std::shared_ptr<spdlog::logger> logger =
      std::make_shared<spdlog::logger>("", begin(sinks), end(sinks));
  spdlog::set_default_logger(logger);
  switch (logLevel) {
    case EB_LOG_LEVEL_DEBUG:
      spdlog::set_level(spdlog::level::debug);
      break;
    case EB_LOG_LEVEL_INFO:
      spdlog::set_level(spdlog::level::info);
      break;
    case EB_LOG_LEVEL_WARN:
      spdlog::set_level(spdlog::level::warn);
      break;
    case EB_LOG_LEVEL_ERROR:
      spdlog::set_level(spdlog::level::err);
      break;
    case EB_LOG_LEVEL_CRITICAL:
      spdlog::set_level(spdlog::level::critical);
      break;
  }

  return EBRESULT_SUCCESS;
}

void EBLogDebug(const char * string) {
  spdlog::debug(string);
}

void EBLogInfo(const char * string) {
  spdlog::info(string);
}

void EBLogWarning(const char * string) {
  spdlog::warn(string);
}

void EBLogError(const char * string) {
  spdlog::error(string);
}

void EBLogCritical(const char * string) {
  spdlog::critical(string);
}