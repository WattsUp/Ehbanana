#include "Ehbanana.h"

#include "EhbananaLog.h"
#include "Version.h"

#include <FruitBowl.h>
#include <Windows.h>

static const EBVersionInfo_t EB_VERSION_INFO = {
    EHBANANA_MAJOR, EHBANANA_MINOR, EHBANANA_PATCH};

const EBVersionInfo_t EBGetVersion() {
  return EB_VERSION_INFO;
}

uint8_t EBAttachCallback(
    const char * uri, const EBInputCallback_t inputCallback) {
  return static_cast<uint8_t>(ResultCode_t::NOT_SUPPORTED);
}

uint8_t EBAttachFileCallback(
    const char * uri, const EBInputFileCallback_t inputFileCallback) {
  return static_cast<uint8_t>(ResultCode_t::NOT_SUPPORTED);
}

uint8_t EBLaunch(EBGUISettings_t guiSettings) {
  return static_cast<uint8_t>(ResultCode_t::NOT_SUPPORTED);
}

bool EBIsDone(bool blocking) {
  return true;
}

uint8_t EBDestroy() {
  return static_cast<uint8_t>(ResultCode_t::NOT_SUPPORTED);
}

uint8_t EBEnqueueOutput(const char * uri, const EBElement_t * elementHead) {
  return static_cast<uint8_t>(ResultCode_t::NOT_SUPPORTED);
}

void EBSetLogger(const EBLogger_t logger) {
  Ehbanana::Logger::Instance()->set(logger);
}