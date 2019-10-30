#ifndef _EHBANANA_H_
#define _EHBANANA_H_

#ifdef COMPILING_DLL
#define EHBANANA_API __declspec(dllexport)

#ifdef GetObject
#undef GetObject
#endif

#else
#define EHBANANA_API __declspec(dllimport)
#endif

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
 * in progress (allows time for browser to load new pages)
 * @param timeoutFirstConnect in seconds to wait before exiting when the first
 * connection is in progress (allows time for browser to boot)
 */
struct EBGUISettings_t {
  char *   configRoot;
  char *   httpRoot;
  uint16_t httpPort            = 0;
  uint8_t  timeoutIdle         = 2;
  uint8_t  timeoutFirstConnect = 20;
};

/**
 * @brief Create a GUI using the settings then open the preferred then default
 * browser
 *
 * @param guiSettings to use to create GUI
 * @return uint8_t zero on success, non-zero on failure
 */
extern "C" EHBANANA_API uint8_t EBLaunch(EBGUISettings_t guiSettings);

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
 */
extern "C" EHBANANA_API void EBSetLogger(const EBLogger_t logger);

#endif /* _EHBANANA_H_ */