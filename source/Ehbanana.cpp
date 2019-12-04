#include "Ehbanana.h"

#include "EhbananaLog.h"
#include "Version.h"
#include "web/Server.h"

#include <FruitBowl.h>

static const EBVersionInfo_t EB_VERSION_INFO = {
    EHBANANA_MAJOR, EHBANANA_MINOR, EHBANANA_PATCH};

Ehbanana::Web::Server server;

const EBVersionInfo_t EBGetVersion() {
  return EB_VERSION_INFO;
}

uint8_t EBAttachCallback(
    const char * uri, const EBInputCallback_t inputCallback) {
  Result result = server.attachCallback(uri, inputCallback);
  if (!result)
    Ehbanana::error((result + "Failed to attach callback").getMessage());
  return static_cast<uint8_t>(result.getCode());
}

uint8_t EBAttachFileCallback(
    const char * uri, const EBInputFileCallback_t inputFileCallback) {
  Result result = server.attachCallback(uri, inputFileCallback);
  if (!result) {
    Ehbanana::error((result + "Failed to attach callback").getMessage());
    return static_cast<uint8_t>(result.getCode());
  }
  return static_cast<uint8_t>(ResultCode_t::SUCCESS);
}

uint8_t EBLaunch(const EBGUISettings_t guiSettings) {
  Result result = server.initialize(guiSettings);
  if (!result) {
    Ehbanana::error((result + "Initializing server").getMessage());
    return static_cast<uint8_t>(result.getCode());
  }

  result = server.start();
  if (!result) {
    Ehbanana::error((result + "Starting server").getMessage());
    return static_cast<uint8_t>(result.getCode());
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
        Ehbanana::error(
            (ResultCode_t::OPEN_FAILED + "Failed to start a web browser")
                .getMessage());
        return static_cast<uint8_t>(ResultCode_t::NO_SYSTEM_CALL);
      }
    }
  }
#else
  if (std::system(("open " + URL).c_str())) {
    Ehbanana::error(
        (ResultCode_t::OPEN_FAILED + "Failed to start a web browser")
            .getMessage());
    return static_cast<uint8_t>(ResultCode_t::NO_SYSTEM_CALL);
  }
#endif

  Ehbanana::info("Web browser opened to " + URL);

  return static_cast<uint8_t>(ResultCode_t::SUCCESS);
}

bool EBIsDone(bool blocking) {
  while (blocking && !server.isDone())
    std::this_thread::sleep_for(millis_t(100));
  return server.isDone();
}

uint8_t EBDestroy() {
  server.stop();
  return static_cast<uint8_t>(ResultCode_t::SUCCESS);
}

uint8_t EBEnqueueOutput(const char * uri, const EBElement_t * elementHead) {
  return static_cast<uint8_t>(ResultCode_t::NOT_SUPPORTED);
}

void EBSetLogger(const EBLogger_t logger) {
  Ehbanana::Logger::Instance()->set(logger);
}

const char * EBErrorName(uint8_t errorCode) {
  if (errorCode > Results::COUNT)
    return "Error code is out of range";
  return Results::MESSAGES[errorCode];
}