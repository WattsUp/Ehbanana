#include "Ehbanana.h"

#include "EhbananaLog.h"
#include "Utils.h"
#include "Version.h"
#include "web/Server.h"

static const EBVersionInfo_t EB_VERSION_INFO = {
    EHBANANA_MAJOR, EHBANANA_MINOR, EHBANANA_PATCH};

Ehbanana::Web::Server server;

const EBVersionInfo_t EBGetVersion() {
  return EB_VERSION_INFO;
}

EBError_t EBAttachCallback(
    const char * uri, const EBInputCallback_t inputCallback) {
  try {
    server.attachCallback(uri, inputCallback);
  } catch (const std::exception & e) {
    Ehbanana::error(e.what());
    return EBError_t::EXCEPTION_OCCURRED;
  }
  return EBError_t::SUCCESS;
}

EBError_t EBAttachFileCallback(
    const char * uri, const EBInputFileCallback_t inputFileCallback) {
  try {
    server.attachCallback(uri, inputFileCallback);
  } catch (const std::exception & e) {
    Ehbanana::error(e.what());
    return EBError_t::EXCEPTION_OCCURRED;
  }
  return EBError_t::SUCCESS;
}

EBError_t EBLaunch(const EBGUISettings_t guiSettings) {
  try {
    server.initialize(guiSettings);
  } catch (const std::exception & e) {
    Ehbanana::error(e.what());
    return EBError_t::INITIALIZATION_FAILED;
  }

  try {
    server.start();
  } catch (const std::exception & e) {
    Ehbanana::error(e.what());
    return EBError_t::START_FAILED;
  }

  std::string URL = "http://";
  URL += server.getDomainName();

#ifdef _WIN32
  std::string command = "--app=\"" + URL + "\"";

  // Try Chrome default install location first
  command =
      "\"\"C:\\Program Files (x86)\\Google\\Chrome\\Application\\chrome.exe\"";
  command += " --app=\"" + URL + "\"\"";
  if (std::system(command.c_str())) {
    Ehbanana::warn("Chrome not found at default installation");
    // Try app data next
    command = "\"\"%LocalAppData%\\Google\\Chrome\\Application\\chrome.exe\"";
    command += " --app=\"" + URL + "\"\"";
    if (std::system(command.c_str())) {
      Ehbanana::warn("Chrome not found in app data");
      // Use default browser last
      if (std::system(("cmd /c start " + URL).c_str())) {
        Ehbanana::error("Failed to start a web browser");
        return EBError_t::PROCESS_START;
      }
    }
  }
#else
  if (std::system(("open " + URL).c_str())) {
    Ehbanana::error("Failed to start a web browser");
    return EBError_t::PROCESS_START;
  }
#endif

  Ehbanana::info("Web browser opened to " + URL);

  return EBError_t::SUCCESS;
}

bool EBIsDone(bool blocking) {
  while (blocking && !server.isDone())
    std::this_thread::sleep_for(millis_t(100));
  return server.isDone();
}

EBError_t EBDestroy() {
  try {
    server.stop();
  } catch (const std::exception & e) {
    Ehbanana::error(e.what());
    return EBError_t::EXCEPTION_OCCURRED;
  }
  return EBError_t::SUCCESS;
}

EBError_t EBEnqueueOutput(
    const char * /* uri */, const EBElement_t * /* elementHead */) {
  return EBError_t::NOT_SUPPORTED;
}

void EBSetLogger(const EBLogger_t logger) {
  Ehbanana::Logger::Instance()->set(logger);
}

const char * EBErrorName(EBError_t errorCode) {
  switch (errorCode) {
    case EBError_t::SUCCESS:
      return "The operation completed successfully";
    case EBError_t::EXCEPTION_OCCURRED:
      return "The operation encountered an exception before completing, see "
             "the error log";
    case EBError_t::INITIALIZATION_FAILED:
      return "The server failed to initialize with the specified gui settings";
    case EBError_t::START_FAILED:
      return "The server failed to start its threads";
    case EBError_t::PROCESS_START:
      return "The operation could not start another process";
    case EBError_t::NOT_SUPPORTED:
      return "The operation is not supported";
    default:
      return "The error code is not recognized";
  }
}