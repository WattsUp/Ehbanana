#include <Ehbanana.h>
#include <Windows.h>

#include "GUI.h"

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
      printf("[Debug] %s\n", string);
      break;
    case EBLogLevel_t::EB_INFO:
      printf("[Info] %s\n", string);
      break;
    case EBLogLevel_t::EB_WARNING:
      printf("[Warn] %s\n", string);
      break;
    case EBLogLevel_t::EB_ERROR:
      printf("[Error] %s\n", string);
      break;
    case EBLogLevel_t::EB_CRITICAL:
      printf("[Critical] %s\n", string);
      break;
  }
}

int WINAPI WinMain(HINSTANCE, HINSTANCE, LPSTR, int) {
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
    std::cout << "Failed to AllocConsole with Win32 error: " << GetLastError()
              << std::endl;
  }

  printf("Ehbanana test starting\n");

  EBSetLogger(logEhbanana);

  Result result = GUI::GUI::Instance()->init();
  if (!result) {
    printf((result + "Initialize GUI").getMessage());
    return static_cast<int>(result.getCode());
  }

  result = GUI::GUI::Instance()->run();
  if (!result) {
    printf((result + "Running GUI").getMessage());
    return static_cast<int>(result.getCode());
  }

  result = GUI::GUI::Instance()->deinit();
  if (!result) {
    printf((result + "Deiniting GUI").getMessage());
    return static_cast<int>(result.getCode());
  }

  printf("Ehbanana test complete");
  return static_cast<int>(ResultCode_t::SUCCESS);
}