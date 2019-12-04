#ifndef _EHBANANA_H_
#define _EHBANANA_H_

#include <stdio.h>

/* stdint.h is not available on older MSVC */
#if defined(_MSC_VER) && (_MSC_VER < 1600) && (!defined(_STDINT)) &&           \
    (!defined(_STDINT_H))
typedef unsigned __int8  uint8_t;
typedef unsigned __int16 uint16_t;
typedef unsigned __int32 uint32_t;
#else
#include <stdint.h>
#endif

#ifdef COMPILING_DLL
#define EHBANANA_API __declspec(dllexport)

#ifdef GetObject
#undef GetObject
#endif

#else
#define EHBANANA_API __declspec(dllimport)
#endif

// Runtime versioning info
struct EBVersionInfo_t {
  uint16_t major;
  uint16_t minor;
  uint16_t patch;
};

/**
 * @brief Get the version info of Ehbanana
 *
 * @return EBVersionInfo_t info struct
 */
extern "C" EHBANANA_API const EBVersionInfo_t EBGetVersion();

/**
 * @brief Process input from the GUI
 *
 * @param id of the triggering element
 * @param value of the triggering element
 */
typedef void(__stdcall * EBInputCallback_t)(
    const char * id, const char * value);

/**
 * @brief Attach a callback to input originating from the uri
 *
 * @param uri of the source page to subscribe to
 * @param inputCallback function
 * @return uint8_t zero on success, non-zero on failure
 */
extern "C" EHBANANA_API uint8_t EBAttachCallback(
    const char * uri, const EBInputCallback_t inputCallback);

/**
 * @brief Process input file from the GUI
 *
 * @param id of the triggering element
 * @param value of the triggering element, usually the filename
 * @param file transferred from the GUI, file will be closed upon returning
 */
typedef void(__stdcall * EBInputFileCallback_t)(
    const char * id, const char * value, FILE * file);

/**
 * @brief Attach a callback to input files originating from the uri
 *
 * @param uri of the source page to subscribe to
 * @param inputFileCallback function
 * @return uint8_t zero on success, non-zero on failure
 */
extern "C" EHBANANA_API uint8_t EBAttachFileCallback(
    const char * uri, const EBInputFileCallback_t inputFileCallback);

/**
 * @brief GUI settings
 *
 * @param configRoot directory containing configuration files
 * @param httpRoot directory containing HTTP top level
 * @param httpPort http server will attempt to open to
 * @param timeoutIdle in seconds to wait before exiting when no connections are
 * in progress (allow time for browser to load new pages)
 */
struct EBGUISettings_t {
  char *   configRoot;
  char *   httpRoot;
  uint16_t httpPort    = 0;
  uint8_t  timeoutIdle = 5;
};

/**
 * @brief Create a GUI using the settings then open the preferred then default
 * browser
 *
 * @param guiSettings to use to create GUI
 * @return uint8_t zero on success, non-zero on failure
 */
extern "C" EHBANANA_API uint8_t EBLaunch(const EBGUISettings_t guiSettings);

/**
 * @brief Check if the GUI is done operating, i.e. all clients are closed
 *
 * @param bool blocking will wait until the GUI is done
 * @return bool true if GUI is done operating, false otherwise
 */
extern "C" EHBANANA_API bool EBIsDone(bool blocking = false);

/**
 * @brief Destroy the GUI
 *
 * @return uint8_t zero on success, non-zero on failure
 */
extern "C" EHBANANA_API uint8_t EBDestroy();

// Type enum for EBElement_t's union
enum class EBValueType_t : uint8_t { UINT64, INT64, DOUBLE, CSTRING };

/**
 * @brief Encapsulation of an output message which modifies the property of an
 * element
 *
 * Multiple elements can be modified together by linking them together as a
 * singly linked list
 *
 * Type must match value or an error will occur. Using EBAddElement is
 * recommended to avoid errors.
 */
struct EBElement_t {
  const char *  id       = nullptr; // HTML id of the element
  const char *  property = nullptr; // to set value for
  EBValueType_t type;               // of encapsulated union
  union {                           // of the property to set
    uint64_t     u;
    int64_t      i;
    double       d;
    const char * c = nullptr;
  } value;
  const EBElement_t * next = nullptr; // Singly linked list of elements
};

/**
 * @brief Enqueue a list of output elements with a desired target page
 *
 * @param uri of the target page, nullptr will broadcast to all pages
 * @param elementHead of the outputElement(s). Elements will be deep copied
 * @return uint8_t zero on success, non-zero on failure
 */
extern "C" EHBANANA_API uint8_t EBEnqueueOutput(
    const char * uri, const EBElement_t * elementHead);

// Logging level enum for filtering messages
enum class EBLogLevel_t : uint8_t {
  EB_DEBUG,
  EB_INFO,
  EB_WARNING,
  EB_ERROR,
  EB_CRITICAL
};

/**
 * @brief Logger callback
 * Print the message string to the destination stream, default: stdout
 *
 * @param EBLogLevel_t log level
 * @param char * string
 */
typedef void(__stdcall * EBLogger_t)(
    const EBLogLevel_t level, const char * string);

/**
 * @brief Set the logger used by Ehbanana
 *
 * If unset: stdout, no filtering
 *
 * @param EBLogger_t logger function, setting nullptr will turn off logging
 */
extern "C" EHBANANA_API void EBSetLogger(const EBLogger_t logger);

/**
 * @brief Get a string representation of the error suitable to present to the
 * user
 *
 * @param uint8_t errorCode to get the string for
 * @return const char * constant string
 */
extern "C" EHBANANA_API const char * EBErrorName(uint8_t errorCode);

#endif /* _EHBANANA_H_ */