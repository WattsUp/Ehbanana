#ifndef _EHBANANA_H_
#define _EHBANANA_H_

#include <ResultCode.h>

#include <Windows.h>
#include <string>

#ifdef COMPILING_DLL
#define EHBANANA_API __declspec(dllexport)
#else
#define EHBANANA_API __declspec(dllimport)
#endif

struct EBGUI;
typedef EBGUI * EBGUI_t;

enum class EBMSGType_t : uint16_t {
  NONE,       // There is no message
  STARTUP,    // The web server has started up
  SHUTDOWN,   // The web server is about to shutdown
  QUIT,       // The web server has quit
  INPUT_FORM, // input from a form element has changed
};

/**
 * @brief Message between front end and back end
 *
 * @param type of the message
 * @param htmlID string tag of the sender or recipient html element
 */
struct EBMessage_t {
  EBGUI_t     gui;
  EBMSGType_t type;
  std::string htmlID;
};

/**
 * @brief Process incoming message from the GUI
 *
 * @param msg to process
 * @return ResultCode_t error code
 */
typedef ResultCode_t(__stdcall * EBGUIProcess_t)(const EBMessage_t &);

/**
 * @brief GUI settings
 *
 * @param guiProcess callback for incoming messages
 * @param configRoot directory containing configuration files
 * @param httpRoot directory containing HTTP top level
 * @param port http server will attempt to open to
 */
struct EBGUISettings_t {
  EBGUIProcess_t guiProcess = nullptr;
  std::string    configRoot;
  std::string    httpRoot;
  uint16_t       port;
};

namespace Web {
class Server;
}

/**
 * @brief GUI object
 *
 * @param settings
 * @param server that executes the GUI
 */
struct EBGUI {
  EBGUISettings_t settings;
  Web::Server *   server;
};

/**
 * @brief Create a GUI using the settings
 *
 * @param guiSettings to use to create GUI
 * @param gui struct to return new GUI
 * @return ResultCode_t error code
 */
extern "C" EHBANANA_API ResultCode_t EBCreateGUI(
    EBGUISettings_t guiSettings, EBGUI_t & gui);

/**
 * @brief Show the GUI by opening the preferred then default browser
 *
 * @param gui to open
 * @return ResultCode_t error code
 */
extern "C" EHBANANA_API ResultCode_t EBShowGUI(EBGUI_t gui);

/**
 * @brief Get the last error's message produced by Ehbanana
 *
 * @return const char * string
 */
extern "C" EHBANANA_API const char * EBGetLastResultMessage();

/**
 * @brief Destroy a GUI
 *
 * @param gui to destroy
 * @return ResultCode_t error code
 */
extern "C" EHBANANA_API ResultCode_t EBDestroyGUI(EBGUI_t gui);

/**
 * @brief Get the next message in the queue
 *
 * Returns ResultCode_tCode_t::SUCCESS when the message type is QUIT
 * Returns ResultCode_tCode_t::NO_OPERATION when the queue is empty
 * Returns ResultCode_tCode_t::INCOMPLETE when the message type is not quit and
 * no other errors occured
 *
 * @param msg to write into
 * @return ResultCode_t error code
 */
extern "C" EHBANANA_API ResultCode_t EBGetMessage(EBMessage_t & msg);

/**
 * @brief Send the message to the appropriate consumers
 *
 * @param msg to dispatch
 * @return ResultCode_t error code
 */
extern "C" EHBANANA_API ResultCode_t EBDispatchMessage(const EBMessage_t & msg);

/**
 * @brief Add a message to the queue
 *
 * @param msg to add
 * @param ResultCode_t error code
 */
extern "C" EHBANANA_API ResultCode_t EBEnqueueMessage(const EBMessage_t & msg);

/**
 * @brief Process incoming message from the GUI in the default method
 *
 * @param msg to process
 * @return ResultCode_t error code
 */
extern "C" EHBANANA_API ResultCode_t EBDefaultGUIProcess(
    const EBMessage_t & msg);

/**
 * @brief Add a quit message to the queue
 *
 */
#define EBEnqueueQuitMessage(gui) (EBEnqueueMessage({gui, EBMSGType_t::QUIT}))

#define EB_LOG_LEVEL_DEBUG 0
#define EB_LOG_LEVEL_INFO 1
#define EB_LOG_LEVEL_WARN 2
#define EB_LOG_LEVEL_ERROR 3
#define EB_LOG_LEVEL_CRITICAL 4

/**
 * @brief Configure logging of the library
 *
 * @param fileName of the log file, nullptr for no logging to file
 * @param rotatingLogs will rotate between 3 files and overwrite the oldest if
 * true, or overwrite the single file if false
 * @param showConsole will open a console output window if true
 * @param logLevel sets the minimum level to log
 * @return ResultCode_t error code
 */
extern "C" EHBANANA_API ResultCode_t EBConfigureLogging(const char * fileName,
    bool rotatingLogs, bool showConsole, uint8_t logLevel);

/**
 * @brief Log a message with debug level
 *
 * @param string to log
 */
extern "C" EHBANANA_API void EBLogDebug(const char * string);

/**
 * @brief Log a message with info level
 *
 * @param string to log
 */
extern "C" EHBANANA_API void EBLogInfo(const char * string);

/**
 * @brief Log a message with warning level
 *
 * @param string to log
 */
extern "C" EHBANANA_API void EBLogWarning(const char * string);

/**
 * @brief Log a message with error level
 *
 * @param string to log
 */
extern "C" EHBANANA_API void EBLogError(const char * string);

/**
 * @brief Log a message with critical level
 *
 * @param string to log
 */
extern "C" EHBANANA_API void EBLogCritical(const char * string);

#endif /* _EHBANANA_H_ */