#include "Ehbanana.h"

#include "MessageOut.h"
#include "web/Server.h"

#include <FruitBowl.h>
#include <Windows.h>
#include <iostream>
#include <list>
#include <stdlib.h>
#include <string>

namespace Ehbanana {
static std::list<EBMessage_t> MessageQueue;
static Result                 lastResult;

/**
 * @brief Logger callback
 * Prints the message string to the destination stream, default: stdout
 *
 * @param EBLogLevel_t log level
 * @param char * string
 */
void __stdcall defaultLogger(const EBLogLevel_t, const char * string) {
  printf(string);
}

static EBLogger_t logger = defaultLogger;

/**
 * @brief Log a message with debug level
 *
 * @param string
 */
void debug(const char * string) {
  logger(EBLogLevel_t::EB_DEBUG, string);
}

/**
 * @brief Log a message with debug level
 *
 * @param string
 */
inline void debug(const std::string & string) {
  debug(string.c_str());
}

/**
 * @brief Log a message with info level
 *
 * @param string
 */
void info(const char * string) {
  logger(EBLogLevel_t::EB_INFO, string);
}

/**
 * @brief Log a message with info level
 *
 * @param string
 */
inline void info(const std::string & string) {
  info(string.c_str());
}

/**
 * @brief Log a message with warning level
 *
 * @param string
 */
void warn(const char * string) {
  logger(EBLogLevel_t::EB_WARNING, string);
}

/**
 * @brief Log a message with warn level
 *
 * @param string
 */
inline void warn(const std::string & string) {
  warn(string.c_str());
}

/**
 * @brief Log a message with error level
 *
 * @param string
 */
void error(const char * string) {
  logger(EBLogLevel_t::EB_ERROR, string);
}

/**
 * @brief Log a message with error level
 *
 * @param string
 */
inline void error(const std::string & string) {
  error(string.c_str());
}

/**
 * @brief Log a message with critical level
 *
 * @param string
 */
void crit(const char * string) {
  logger(EBLogLevel_t::EB_CRITICAL, string);
}

/**
 * @brief Log a message with critical level
 *
 * @param string
 */
inline void crit(const std::string & string) {
  crit(string.c_str());
}

/**
 * @brief Log a message using format specifier
 *
 * @param level to log at
 * @param format string
 * @param ... args
 */
void log(EBLogLevel_t level, const char * format, ...) {
  int     bufLen = 128;
  char *  buf    = new char[bufLen];
  va_list args;
  va_start(args, format);
  int n = vsnprintf(buf, bufLen, format, args);
  if (n > bufLen) {
    delete buf;
    bufLen = n + 1;
    buf    = new char[bufLen];
    n      = vsnprintf(buf, bufLen, format, args);
    if (n > bufLen) {
      crit("Could not print format string");
      va_end(args);
      delete buf;
      return;
    }
  }
  va_end(args);
  logger(level, buf);
  delete buf;
}

} // namespace Ehbanana

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
    Ehbanana::error(
        "Failed to create process with command \"" + command + "\"");
    delete buf;
    return ResultCode_t::BAD_COMMAND + ("Create process: " + command);
  }
  delete buf;
  return ResultCode_t::SUCCESS;
}

ResultCode_t EBCreateGUI(EBGUISettings_t guiSettings, EBGUI_t & gui) {
  // Construct a new EBGUI object to store settings and the server
  Result result = EBDestroyGUI(gui);
  if (!result) {
    Ehbanana::error(
        (result + "Desroying GUI before creating a new one").getMessage());
    return result.getCode();
  }
  gui = new EBGUI();

  if (guiSettings.guiProcess == nullptr) {
    delete gui;
    Ehbanana::error((result + "guiProcess is nullptr").getMessage());
    return result.getCode();
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
      if (!result) {
        // No hope
        Ehbanana::error((result + "Configuring new server").getMessage());
        return result.getCode();
      }
    } else {
      Ehbanana::error((result + "Configuring new server").getMessage());
      return result.getCode();
    }
  }

  result = gui->server->initializeSocket("127.0.0.1", guiSettings.httpPort);
  if (!result) {
    delete gui->server;
    delete gui;
    Ehbanana::error((result + "Initializing server's socket").getMessage());
    return result.getCode();
  }

  gui->server->start();

  result = EBEnqueueMessage({gui, EBMSGType_t::STARTUP});
  if (!result) {
    delete gui->server;
    delete gui;
    Ehbanana::error((result + "Enqueueing STARTUP message").getMessage());
    return result.getCode();
  }

  gui->settings = guiSettings;
  return ResultCode_t::SUCCESS;
}

ResultCode_t EBShowGUI(EBGUI_t gui) {
  // Ensure system calls is available
  if (!std::system(nullptr)) {
    Ehbanana::error(
        (ResultCode_t::NO_SYSTEM_CALL + "Showing GUI").getMessage());
    return ResultCode_t::NO_SYSTEM_CALL;
  }

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
      if (!EBCreateProcess(command, &browser)) {
        Ehbanana::error(
            (ResultCode_t::OPEN_FAILED + "Failed to start a web browser")
                .getMessage());
        return ResultCode_t::NO_SYSTEM_CALL;
      }
    }
  }

  // Wait for the process to finish spawning the browser up to 1s
  WaitForSingleObject(browser.hProcess, 1000);
  CloseHandle(browser.hProcess);
  CloseHandle(browser.hThread);

  // spdlog::info("Web browser opened to {}", URL);

  return ResultCode_t::SUCCESS;
}

ResultCode_t EBDestroyGUI(EBGUI_t gui) {
  if (gui != nullptr)
    delete gui->server;
  delete gui;
  return ResultCode_t::SUCCESS;
}

ResultCode_t EBGetMessage(EBMessage_t & msg) {
  if (Ehbanana::MessageQueue.empty()) {
    msg.type = EBMSGType_t::NONE;
    return ResultCode_t::NO_OPERATION;
  }
  msg = Ehbanana::MessageQueue.front();
  Ehbanana::MessageQueue.pop_front();
  if (msg.type == EBMSGType_t::QUIT) {
    msg.gui->server->stop();
    return ResultCode_t::SUCCESS;
  }
  return ResultCode_t::INCOMPLETE;
}

ResultCode_t EBDispatchMessage(const EBMessage_t & msg) {
  Result result = (msg.gui->settings.guiProcess)(msg);
  if (!result) {
    Ehbanana::error((result + "Dispatched message to guiProcess").getMessage());
    return result.getCode();
  }
  return ResultCode_t::SUCCESS;
}

ResultCode_t EBEnqueueMessage(const EBMessage_t & msg) {
  Ehbanana::MessageQueue.push_back(msg);
  return ResultCode_t::SUCCESS;
}

ResultCode_t EBDefaultGUIProcess(const EBMessage_t & msg) {
  Ehbanana::error(
      (ResultCode_t::NOT_SUPPORTED + "Default GUI process").getMessage());
  return ResultCode_t::NOT_SUPPORTED;
}

ResultCode_t EBMessageOutCreate(EBGUI_t gui) {
  if (gui->currentMessageOut == nullptr)
    delete gui->currentMessageOut;

  gui->currentMessageOut = new MessageOut();
  return ResultCode_t::SUCCESS;
}

ResultCode_t EBMessageOutSetHref(EBGUI_t gui, const char * href) {
  if (gui->currentMessageOut == nullptr) {
    Ehbanana::error(
        (ResultCode_t::INVALID_DATA + "currentMessageOut is nullptr")
            .getMessage());
    return ResultCode_t::INVALID_DATA;
  }
  Result result = gui->currentMessageOut->setHref(href);
  if (!result) {
    Ehbanana::error((result + "Setting message out href").getMessage());
    return result.getCode();
  }
  return ResultCode_t::SUCCESS;
}

ResultCode_t EBMessageOutSetProp(
    EBGUI_t gui, const char * id, const char * name, const char * value) {
  if (gui->currentMessageOut == nullptr) {
    Ehbanana::error(
        (ResultCode_t::INVALID_DATA + "currentMessageOut is nullptr")
            .getMessage());
    return ResultCode_t::INVALID_DATA;
  }
  Result result = gui->currentMessageOut->setProperty(id, name, value);
  if (!result) {
    Ehbanana::error((result + "Setting message out property").getMessage());
    return result.getCode();
  }
  return ResultCode_t::SUCCESS;
}

ResultCode_t EBMessageOutEnqueue(EBGUI_t gui) {
  if (gui->currentMessageOut == nullptr) {
    Ehbanana::error(
        (ResultCode_t::INVALID_DATA + "currentMessageOut is nullptr")
            .getMessage());
    return ResultCode_t::INVALID_DATA;
  }
  if (!gui->currentMessageOut->isEnqueued())
    gui->server->enqueueOutput(gui->currentMessageOut->getString());
  return ResultCode_t::SUCCESS;
}

void EBSetLogger(const EBLogger_t logger) {
  Ehbanana::logger = logger;
}