#ifndef _EHBANANA_H_
#define _EHBANANA_H_

#include <FruitBowl.h>

#ifdef EB_USE_STD_STRING
#include <string>
#endif

#ifdef GetObject
#undef GetObject
#endif

#ifdef COMPILING_DLL
#define EHBANANA_API __declspec(dllexport)
#else
#define EHBANANA_API __declspec(dllimport)
#endif

struct EBGUI;
typedef EBGUI * EBGUI_t;

enum class EBMSGType_t : uint8_t {
  NONE,     // There is no message
  STARTUP,  // The web server has started up
  SHUTDOWN, // The web server is about to shutdown
  QUIT,     // The web server has quit
  INPUT,    // An input element has changed
};

/**
 * @brief Message from front end to back end
 *
 * @param gui object to alert
 * @param type of the message
 * @param href URL of the webpage sender or receiver
 * @param id of the originating html element
 * @param value of the originating html element
 * @param file handle when element is a file
 * @param fileSize if handle is valid
 */
struct EBMessage_t {
  EBGUI_t     gui;
  EBMSGType_t type;

  Hash   href;
  Hash   id;
  Hash   value;
  FILE * file;
  size_t fileSize;
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
 * @param httpPort http server will attempt to open to
 */
struct EBGUISettings_t {
  EBGUIProcess_t guiProcess = nullptr;
  char *         configRoot;
  char *         httpRoot;
  uint16_t       httpPort = 0;
};

namespace Web {
class Server;
}

class MessageOut;

/**
 * @brief GUI object
 *
 * @param settings
 * @param server that executes the GUI
 */
struct EBGUI {
  EBGUISettings_t settings;
  Web::Server *   server            = nullptr;
  MessageOut *    currentMessageOut = nullptr;
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
 * no other errors occurred
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

/**
 * @brief Create an outgoing message for the GUI
 *
 * @param gui to create the message for
 * @return ResultCode_t
 */
extern "C" EHBANANA_API ResultCode_t EBMessageOutCreate(EBGUI_t gui);

/**
 * @brief Set the href for the current outgoing message for the GUI
 *
 * @param gui to set the href for
 * @param href string to set
 * @return ResultCode_t
 */
extern "C" EHBANANA_API ResultCode_t EBMessageOutSetHref(
    EBGUI_t gui, const char * href);

#ifdef EB_USE_STD_STRING
/**
 * @brief Set the href for the current outgoing message for the GUI
 *
 * @param gui to set the href for
 * @param href string to set
 * @return ResultCode_t
 */
inline ResultCode_t EBMessageOutSetHref(EBGUI_t gui, std::string href) {
  return EBMessageOutSetHref(gui, href.c_str());
}
#endif

/**
 * @brief Set a property for the current outgoing message for the GUI
 *
 * @param gui to set the property for
 * @param id of the HTML element
 * @param name of the property
 * @param value of the property
 * @return ResultCode_t
 */
extern "C" EHBANANA_API ResultCode_t EBMessageOutSetProp(
    EBGUI_t gui, const char * id, const char * name, const char * value);

#ifdef EB_USE_STD_STRING
/**
 * @brief Set a property for the current outgoing message for the GUI
 *
 * @param gui to set the property for
 * @param id of the HTML element
 * @param name of the property
 * @param value of the property
 * @return ResultCode_t
 */
inline ResultCode_t EBMessageOutSetProp(
    EBGUI_t gui, std::string id, std::string name, std::string value) {
  return EBMessageOutSetProp(gui, id.c_str(), name.c_str(), value.c_str());
}
#endif

/**
 * @brief Enqueue the current outgoing message for the GUI
 *
 * @param gui to enqueue the message for
 * @return ResultCode_t
 */
extern "C" EHBANANA_API ResultCode_t EBMessageOutEnqueue(EBGUI_t gui);

enum class EBLogLevel_t : uint8_t {
  EB_DEBUG,
  EB_INFO,
  EB_WARNING,
  EB_ERROR,
  EB_CRITICAL
};

/**
 * @brief Logger callback
 * Prints the message string to the destination stream, default: stdout
 *
 * @param char * string
 * @param EBLogLevel_t log level
 */
typedef void(__stdcall * EBLogger)(
    const EBLogLevel_t level, const char * string);

/**
 * @brief Set the logger used by Ehbanana
 *
 * If unset: stdout, no filtering
 */
extern "C" EHBANANA_API void EBSetLogger(const EBLogger logger);

#endif /* _EHBANANA_H_ */