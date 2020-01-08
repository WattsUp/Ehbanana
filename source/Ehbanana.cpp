#include "Ehbanana.h"

#include "EhbananaLog.h"
#include "MessageOut.h"
#include "Stream.h"
#include "Utils.h"
#include "Version.h"
#include "web/Server.h"

#include <algorithm>
#include <memory>

static const EBVersionInfo_t EB_VERSION_INFO = {
    EHBANANA_MAJOR, EHBANANA_MINOR, EHBANANA_PATCH};

std::shared_ptr<Ehbanana::Web::Server> server;

const EBVersionInfo_t EBGetVersion() {
  return EB_VERSION_INFO;
}

EBError_t EBAttachCallback(
    const char * uri, const EBInputCallback_t inputCallback) {
  if (server == nullptr) {
    Ehbanana::error("No server created, use EBCreate");
    return EBError_t::NO_SERVER_CREATED;
  }

  try {
    server->attachCallback(uri, inputCallback);
  } catch (const std::exception & e) {
    Ehbanana::error(e.what());
    return EBError_t::EXCEPTION_OCCURRED;
  }
  return EBError_t::SUCCESS;
}

EBError_t EBStreamRead(EBStream_t stream, uint8_t * buf) {
  if (stream == nullptr || buf == nullptr) {
    Ehbanana::error("Buffer is invalid");
    return EBError_t::EXCEPTION_OCCURRED;
  }
  Ehbanana::Stream * obj = (Ehbanana::Stream *)stream;
  try {
    *buf = obj->read();
  } catch (const std::underflow_error & /* e */) {
    if (obj->eof())
      return EBError_t::END_OF_FILE; // TODO add block read/writes
    return EBError_t::BUFFER_EMPTY;
  } catch (const std::exception & e) {
    Ehbanana::error(e.what());
    return EBError_t::EXCEPTION_OCCURRED;
  }
  return EBError_t::SUCCESS;
}

EBError_t EBStreamWrite(EBStream_t stream, const uint8_t buf) {
  if (stream == nullptr) {
    Ehbanana::error("Buffer is invalid");
    return EBError_t::EXCEPTION_OCCURRED;
  }
  try {
    ((Ehbanana::Stream *)stream)->write(buf);
  } catch (const std::exception & e) {
    Ehbanana::debug(e.what());
    return EBError_t::EXCEPTION_OCCURRED;
  }
  return EBError_t::SUCCESS;
}

EBError_t EBAttachInputFileCallback(
    const char * uri, const EBInputFileCallback_t inputFileCallback) {
  if (server == nullptr) {
    Ehbanana::error("No server created, use EBCreate");
    return EBError_t::NO_SERVER_CREATED;
  }

  try {
    server->attachCallback(uri, inputFileCallback);
  } catch (const std::exception & e) {
    Ehbanana::error(e.what());
    return EBError_t::EXCEPTION_OCCURRED;
  }
  return EBError_t::SUCCESS;
}

EBError_t EBSetOutputFileCallback(
    const EBOutputFileCallback_t outputFileCallback) {
  if (server == nullptr) {
    Ehbanana::error("No server created, use EBCreate");
    return EBError_t::NO_SERVER_CREATED;
  }

  try {
    server->setOutputCallback(outputFileCallback);
  } catch (const std::exception & e) {
    Ehbanana::error(e.what());
    return EBError_t::EXCEPTION_OCCURRED;
  }
  return EBError_t::SUCCESS;
}

EBError_t EBCreate(const EBGUISettings_t guiSettings) {
  try {
    server = std::make_shared<Ehbanana::Web::Server>(guiSettings);
  } catch (const std::exception & e) {
    Ehbanana::error(e.what());
    return EBError_t::INITIALIZATION_FAILED;
  }
  return EBError_t::SUCCESS;
}

EBError_t EBLaunch() {
  if (server == nullptr) {
    Ehbanana::error("No server created, use EBCreate");
    return EBError_t::NO_SERVER_CREATED;
  }

  try {
    server->start();
  } catch (const std::exception & e) {
    Ehbanana::error(e.what());
    return EBError_t::START_FAILED;
  }

  std::string URL = "http://";
  URL += server->getDomainName();

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
  if (server == nullptr) {
    Ehbanana::error("No server created, use EBCreate");
    return true;
  }

  while (blocking && !server->isDone())
    std::this_thread::sleep_for(Ehbanana::millis_t(100));
  return server->isDone();
}

EBError_t EBDestroy() {
  if (server == nullptr) {
    Ehbanana::error("No server created, use EBCreate");
    return EBError_t::NO_SERVER_CREATED;
  }

  try {
    server->stop();
    server = nullptr;
  } catch (const std::exception & e) {
    Ehbanana::error(e.what());
    return EBError_t::EXCEPTION_OCCURRED;
  }
  return EBError_t::SUCCESS;
}

EBError_t EBCreateOutput(const char * uri, EBOutput_t * output) {
  try {
    *output = new Ehbanana::MessageOut(uri);
  } catch (const std::exception & e) {
    Ehbanana::error(e.what());
    return EBError_t::EXCEPTION_OCCURRED;
  }
  return EBError_t::SUCCESS;
}

EBError_t EBAddOutput(const EBOutput_t output, const char * id,
    const char * property, const EBValueType_t type, const void * value) {
  if (output == nullptr || id == nullptr || property == nullptr ||
      value == nullptr) {
    Ehbanana::error("One or more arguments are null");
    return EBError_t::NULL_ARGUMENT;
  }

  try {
    Ehbanana::MessageOut * messageOut =
        reinterpret_cast<Ehbanana::MessageOut *>(output);
    switch (type) {
      case EBValueType_t::UINT8:
        messageOut->add(id, property, *(const uint8_t *)value);
        break;
      case EBValueType_t::INT8:
        messageOut->add(id, property, *(const int8_t *)value);
        break;
      case EBValueType_t::UINT16:
        messageOut->add(id, property, *(const uint16_t *)value);
        break;
      case EBValueType_t::INT16:
        messageOut->add(id, property, *(const int16_t *)value);
        break;
      case EBValueType_t::UINT32:
        messageOut->add(id, property, *(const uint32_t *)value);
        break;
      case EBValueType_t::INT32:
        messageOut->add(id, property, *(const int32_t *)value);
        break;
      case EBValueType_t::UINT64:
        messageOut->add(id, property, *(const uint64_t *)value);
        break;
      case EBValueType_t::INT64:
        messageOut->add(id, property, *(const int64_t *)value);
        break;
      case EBValueType_t::FLOAT:
        messageOut->add(id, property, *(const float *)value);
        break;
      case EBValueType_t::DOUBLE:
        messageOut->add(id, property, *(const double *)value);
        break;
      case EBValueType_t::CSTRING:
        messageOut->add(id, property, (const char *)value);
        break;
      default:
        throw std::exception("Value type is not recognized");
        break;
    }
  } catch (const std::exception & e) {
    Ehbanana::error(e.what());
    return EBError_t::EXCEPTION_OCCURRED;
  }
  return EBError_t::SUCCESS;
}

EBError_t EBEnqueueOutput(const EBOutput_t output) {
  if (server == nullptr) {
    Ehbanana::error("No server created, use EBCreate");
    return EBError_t::NO_SERVER_CREATED;
  }

  if (output == nullptr) {
    Ehbanana::error("One or more arguments are null");
    return EBError_t::NULL_ARGUMENT;
  }

  try {
    std::shared_ptr<Ehbanana::MessageOut> messageOut(
        (Ehbanana::MessageOut *)output);
    server->enqueueOutput(messageOut);
  } catch (const std::exception & e) {
    Ehbanana::error(e.what());
    return EBError_t::EXCEPTION_OCCURRED;
  }
  return EBError_t::SUCCESS;
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
    case EBError_t::NO_SERVER_CREATED:
      return "The operation requires a server to be created first, see "
             "EBCreate";
    case EBError_t::END_OF_FILE:
      return "The stream has reached the end of the file, no more data to read";
    case EBError_t::BUFFER_EMPTY:
      return "The stream has reached the end of its internal buffer, wait for "
             "more data";
    case EBError_t::BUFFER_FULL:
      return "The stream has filled its internal buffer, wait for data to be "
             "consumed";
    case EBError_t::NULL_ARGUMENT:
      return "One or more arguments is null and not allowed";
    default:
      return "The error code is not recognized";
  }
}