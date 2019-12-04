#include <Ehbanana.h>
#include <Windows.h>

#include <stdio.h>

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
      break;
    case EBLogLevel_t::EB_INFO:
      printf("[Info]     %s\n", string);
      break;
    case EBLogLevel_t::EB_WARNING:
      printf("[Warn]     %s\n", string);
      break;
    case EBLogLevel_t::EB_ERROR:
      printf("[Error]    %s\n", string);
      break;
    case EBLogLevel_t::EB_CRITICAL:
      printf("[Critical] %s\n", string);
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

  printf("Ehbanana test starting\n");

  EBSetLogger(logEhbanana);

  uint8_t error = EBAttachCallback("/", callbackRoot);
  if (error) {
    MessageBoxA(NULL, EBErrorName(error), "Error", MB_OK);
    printf("Failed to launch GUI");
    return error;
  }

  EBGUISettings_t guiSettings {"test/config", "test/http"};

  error = EBLaunch(guiSettings);
  if (error) {
    MessageBoxA(NULL, EBErrorName(error), "Error", MB_OK);
    printf("Failed to launch GUI");
    return error;
  }

  // Block until the GUI is done
  EBIsDone(true);

  error = EBDestroy();
  if (error) {
    MessageBoxA(NULL, EBErrorName(error), "Error", MB_OK);
    printf("Failed to destroy GUI");
    return error;
  }

  printf("Ehbanana test complete");
  return 0;
}