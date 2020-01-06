#include <Ehbanana.h>
#include <Windows.h>

#include <chrono>
#include <stdio.h>
#include <string>
#include <thread>

FILE * logFile;

/**
 * @brief Logger callback
 * Prints the message string to the destination stream, default: stdout
 *
 * @param EBLogLevel_t log level
 * @param char * string
 */
void __stdcall logEhbanana(const EBLogLevel_t level, const char * string) {
  switch (level) {
    case EBLogLevel_t::EB_DEBUG:
      printf("[Debug]    %s\n", string);
      fprintf(logFile, "[Debug]    %s\n", string);
      break;
    case EBLogLevel_t::EB_INFO:
      printf("[Info]     %s\n", string);
      fprintf(logFile, "[Info]     %s\n", string);
      break;
    case EBLogLevel_t::EB_WARNING:
      printf("[Warn]     %s\n", string);
      fprintf(logFile, "[Warn]     %s\n", string);
      break;
    case EBLogLevel_t::EB_ERROR:
      printf("[Error]    %s\n", string);
      fprintf(logFile, "[Error]    %s\n", string);
      break;
    case EBLogLevel_t::EB_CRITICAL:
      printf("[Critical] %s\n", string);
      fprintf(logFile, "[Critical] %s\n", string);
      break;
  }
}

/**
 * @brief Process input from the GUI page "/"
 *
 * @param id of the triggering element
 * @param value of the triggering element
 */
void __stdcall callbackRoot(const char * id, const char * value) {
  printf("Callback from %s with %s\n", id, value);
}

/**
 * @brief Process input from the GUI page "/"
 *
 * @param id of the triggering element
 * @param value of the triggering element
 */
void __stdcall callbackRootFile(
    const char * id, const char * value, EBBuffer_t * file) {
  printf("Callback file from %s with %s\n", id, value);
}

void __stdcall callbackOutputFile(
    const char * uri, const Query_t * queries, EBBuffer_t * file) {
  printf("Callback 404 for: %s", uri);
}

int WINAPI WinMain(HINSTANCE, HINSTANCE, LPSTR, int) {
  // Create a console for logging errors
  if (AllocConsole()) {
    HWND hwnd = GetConsoleWindow();
    if (hwnd != NULL) {
      HMENU hMenu = GetSystemMenu(hwnd, FALSE);
      if (hMenu != NULL)
        DeleteMenu(hMenu, SC_CLOSE, MF_BYCOMMAND);
    }
    freopen_s((FILE **)stdout, "CONOUT$", "w", stdout);
  } else {
    MessageBoxA(NULL, "Log console initialization failed", "Error", MB_OK);
    printf("Failed to AllocConsole with Win32 error: %d\n", GetLastError());
  }
  errno_t err = fopen_s(&logFile, "log.log", "a");
  if (err)
    MessageBoxA(NULL, "Log file failed to open log", "Error", MB_OK);

  printf("Ehbanana test starting\n");

  EBSetLogger(logEhbanana);

  EBGUISettings_t guiSettings;
  guiSettings.configRoot = "test/config";
  guiSettings.httpRoot   = "test/http";

  EBError_t error = EBCreate(guiSettings);
  if (EB_FAILED(error)) {
    printf("Failed to create GUI");
    MessageBoxA(NULL, EBErrorName(error), "Error", MB_OK);
    _fcloseall();
    return static_cast<uint8_t>(error);
  }

  error = EBAttachCallback("/", callbackRoot);
  if (EB_FAILED(error)) {
    printf("Failed to attach callback to GUI");
    MessageBoxA(NULL, EBErrorName(error), "Error", MB_OK);
    _fcloseall();
    return static_cast<uint8_t>(error);
  }

  error = EBAttachInputFileCallback("/", callbackRootFile);
  if (EB_FAILED(error)) {
    printf("Failed to attach callback to GUI");
    MessageBoxA(NULL, EBErrorName(error), "Error", MB_OK);
    _fcloseall();
    return static_cast<uint8_t>(error);
  }

  error = EBSetOutputFileCallback(callbackOutputFile);
  if (EB_FAILED(error)) {
    printf("Failed to attach callback to GUI");
    MessageBoxA(NULL, EBErrorName(error), "Error", MB_OK);
    _fcloseall();
    return static_cast<uint8_t>(error);
  }

  error = EBLaunch();
  if (EB_FAILED(error)) {
    printf("Failed to launch GUI");
    MessageBoxA(NULL, EBErrorName(error), "Error", MB_OK);
    _fcloseall();
    return static_cast<uint8_t>(error);
  }

  // Block until the GUI is done
  EBIsDone(true);

  error = EBDestroy();
  if (EB_FAILED(error)) {
    printf("Failed to destroy GUI");
    MessageBoxA(NULL, EBErrorName(error), "Error", MB_OK);
    _fcloseall();
    return static_cast<uint8_t>(error);
  }

  printf("Ehbanana test complete");
  _fcloseall();
  return 0;
}