#ifndef _EHBANANA_H_
#define _EHBANANA_H_

#include <Windows.h>

#include "Result.h"

#ifdef COMPILING_DLL
#define EHBANANA_API __declspec(dllexport)
#else
#define EHBANANA_API __declspec(dllimport)
#endif

enum class EBMSGType_t : uint16_t {
  INPUT_FORM // input from a form element has changed
};

/**
 * @brief Message between front end and back end
 *
 * @param type of the message
 * @param htmlID string tag of the sender or recipient html element
 */
struct EBMessage_t {
  EBMSGType_t type;
  char *      htmlID;
};

/**
 * @brief Process incoming message from the GUI
 *
 * @param msg to process
 * @return EBResult_t error code
 */
typedef EBResult_t(__stdcall * EBGUIProcess_t)(EBMessage_t);

/**
 * @brief GUI settings
 *
 * @param guiProcess callback for incoming messages
 * @param configRoot directory containing configuration files
 * @param httpRoot directory containing HTTP top level
 */
struct EBGUISettings_t {
  EBGUIProcess_t guiProcess;
  const char *   configRoot;
  const char *   httpRoot;
};

/**
 * @brief GUI object
 *
 * @param settings
 */
struct EBGUI {
  EBGUISettings_t settings;
};

typedef EBGUI * EBGUI_t;

/**
 * @brief Create a GUI using the settings
 *
 * @param guiSettings to use to create GUI
 * @return EBGUI_t gui or nullptr if failed, check EBGetLastResult for reason
 */
extern "C" EHBANANA_API EBGUI_t EBCreateGUI(EBGUISettings_t guiSettings);

/**
 * @brief Show the GUI by opening the preferred then default browser
 *
 * @param gui to open
 * @return EBResult_t error code
 */
extern "C" EHBANANA_API EBResult_t EBShowGUI(EBGUI_t gui);

/**
 * @brief Get the last error produced by Ehbanana
 *
 * @return EBResult_t error code
 */
extern "C" EHBANANA_API EBResult_t EBGetLastResult();

/**
 * @brief Set the last error produced by Ehbanana
 *
 * @param result to set
 * @return last result
 */
EBResult_t EBSetLastResult(EBResult_t result);

#endif /* _EHBANANA_H_ */