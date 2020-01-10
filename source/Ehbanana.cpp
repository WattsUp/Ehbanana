#include "Ehbanana.h"

#include "EhbananaLog.h"
#include "MessageOut.h"
#include "Stream.h"
#include "Utils.h"
#include "Version.h"
#include "web/Server.h"

#include <algorithm>
#include <memory>

#ifdef _WIN32
#include <Windows.h>
#endif /* _WIN32 */

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
      return EBError_t::END_OF_FILE;
    return EBError_t::BUFFER_EMPTY;
  } catch (const std::exception & e) {
    Ehbanana::error(e.what());
    return EBError_t::EXCEPTION_OCCURRED;
  }
  return EBError_t::SUCCESS;
}

EBError_t EBStreamReadBlock(EBStream_t stream, uint8_t * buf, size_t * length) {
  if (stream == nullptr || buf == nullptr) {
    Ehbanana::error("Buffer is invalid");
    return EBError_t::EXCEPTION_OCCURRED;
  }
  Ehbanana::Stream * obj = (Ehbanana::Stream *)stream;
  try {
    *length = obj->read(buf, *length);
    if (*length == 0) {
      if (obj->eof())
        return EBError_t::END_OF_FILE;
      return EBError_t::BUFFER_EMPTY;
    }
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

EBError_t EBStreamWriteBlock(
    EBStream_t stream, const uint8_t * buf, size_t length) {
  if (stream == nullptr) {
    Ehbanana::error("Buffer is invalid");
    return EBError_t::EXCEPTION_OCCURRED;
  }
  try {
    ((Ehbanana::Stream *)stream)->write(buf, length);
  } catch (const std::exception & e) {
    Ehbanana::debug(e.what());
    return EBError_t::EXCEPTION_OCCURRED;
  }
  return EBError_t::SUCCESS;
}

EBError_t EBStreamFinish(EBStream_t stream) {
  if (stream == nullptr) {
    Ehbanana::error("Buffer is invalid");
    return EBError_t::EXCEPTION_OCCURRED;
  }
  ((Ehbanana::Stream *)stream)->setEOF(true);
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

EBError_t EBSet404Callback(const EB404Callback_t callback404) {
  return EBError_t::NOT_SUPPORTED;
  // if (server == nullptr) {
  //   Ehbanana::error("No server created, use EBCreate");
  //   return EBError_t::NO_SERVER_CREATED;
  // }

  // try {
  //   server->set404Callback(callback404);
  // } catch (const std::exception & e) {
  //   Ehbanana::error(e.what());
  //   return EBError_t::EXCEPTION_OCCURRED;
  // }
  // return EBError_t::SUCCESS;
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

/**
 * @brief Spawn a process with the application and arguments
 * Call will not block
 *
 * @param envVar environment variable to prepend to application, null to ignore
 * @param application
 * @param arguments
 * @return bool true if process has been spawned, false otherwise
 */
bool spawnProcess(
    const char * envVar, const char * application, const char * arguments) {
#ifdef _WIN32
  STARTUPINFO         si;
  PROCESS_INFORMATION pi;

  ZeroMemory(&si, sizeof(si));
  si.cb = sizeof(si);
  ZeroMemory(&pi, sizeof(pi));

  std::string command = "\"";

  if (envVar != nullptr) {
    char buf[64];
    if (GetEnvironmentVariableA(envVar, buf, 128) == 0) {
      Ehbanana::error("Environment variable not found");
      return false;
    }
    command += buf;
    command += "\\";
  }
  command += application;
  command += "\" ";
  command += arguments;

  // Start the child process
  // Arguments is casted to non-const but only unicode version may modify it
  if (!CreateProcessA(NULL, const_cast<char *>(command.c_str()),
          NULL,  // Process handle not inheritable
          NULL,  // Thread handle not inheritable
          FALSE, // Set handle inheritance to FALSE
          0,     // No creation flags
          NULL,  // Use parent's environment block
          NULL,  // Use parent's starting directory
          &si,   // Pointer to STARTUPINFO structure
          &pi)   // Pointer to PROCESS_INFORMATION structure
  ) {
    Ehbanana::error("CreateProcess failed: " + std::to_string(GetLastError()));
    CloseHandle(pi.hProcess);
    CloseHandle(pi.hThread);
    return false;
  }
  CloseHandle(pi.hProcess);
  CloseHandle(pi.hThread);

#else  /* _WIN32 */
  return false; // TODO open process on not Windows
#endif /* _WIN32 */
  return true;
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

  std::string URL = "http://" + server->getDomainName();

#ifdef _WIN32
  std::string command = "--app=\"" + URL + "\"";

  if (!spawnProcess("ProgramFiles(x86)",
          "Google\\Chrome\\Application\\chrome.exe", command.c_str())) {
    Ehbanana::warn("Chrome not found at default installation");
    if (!spawnProcess("LocalAppData", "Google\\Chrome\\Application\\chrome.exe",
            command.c_str())) {
      Ehbanana::warn("Chrome not found in app data");
      if (!spawnProcess(nullptr, "cmd", ("/c start " + URL).c_str())) {
        Ehbanana::error("Failed to start a web browser");
        return EBError_t::PROCESS_START;
      }
    }
  }
#else  /* _WIN32 */
  if (!spawnProcess("open", URL)) {
    Ehbanana::error("Failed to start a web browser");
    return EBError_t::PROCESS_START;
  }
#endif /* _WIN32 */

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