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

enum class EBError_t : uint8_t {
  SUCCESS               = 0x00,
  EXCEPTION_OCCURRED    = 0x01,
  INITIALIZATION_FAILED = 0x02,
  START_FAILED          = 0x03,
  PROCESS_START         = 0x04,
  NOT_SUPPORTED         = 0x05
};

#define EB_FAILED(error) (error != EBError_t::SUCCESS)

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
 * @return EBError_t zero on success, non-zero on failure
 */
extern "C" EHBANANA_API EBError_t EBAttachCallback(
    const char * uri, const EBInputCallback_t inputCallback);

/**
 * @brief A buffer to transfer files to/from the GUI
 * Use EBBufferRead and EBBufferWrite to read/write
 *
 */
struct EBBuffer_t {
  uint8_t * buf      = nullptr;
  uint16_t  head     = 0;
  uint16_t  tail     = 0;
  uint16_t  size     = 0;
  bool      complete = false;
};

/**
 * @brief Process input file from the GUI
 *
 * @param id of the triggering element
 * @param value of the triggering element, usually the filename
 * @param file transferred from the GUI, complete flag is set once done
 * receiving
 */
typedef void(__stdcall * EBInputFileCallback_t)(
    const char * id, const char * value, EBBuffer_t * file);

/**
 * @brief Attach a callback to input files originating from the uri
 *
 * @param uri of the source page to subscribe to
 * @param inputFileCallback function
 * @return EBError_t zero on success, non-zero on failure
 */
extern "C" EHBANANA_API EBError_t EBAttachInputFileCallback(
    const char * uri, const EBInputFileCallback_t inputFileCallback);

/**
 * @brief Singly linked list of queries
 *  127.0.0.1/?a=1&food=banana becomes {[a, 1], [food, banana]}
 *
 * @param name of the query
 * @param value of the query
 * @param next query in the linked list, nullptr if last
 */
struct Query_t {
  const char *    name;
  const char *    value;
  const Query_t * next = nullptr;
};

/**
 * @brief Process output file to the GUI
 * Set the complete flag if file was properly handled and done writing
 * Exit the function before setting the complete flag if the file is not to be
 * found
 *
 * @param uri of the file being downloaded
 * @param queries following the uri
 * @param file to send to the GUI, set the complete flag once done writing
 */
typedef void(__stdcall * EBOutputFileCallback_t)(
    const char * uri, const Query_t * queries, EBBuffer_t * file);

/**
 * @brief Set the callback to output files not found in the HTTP directory
 *
 * @param outputFileCallback function
 * @return EBError_t zero on success, non-zero on failure
 */
extern "C" EHBANANA_API EBError_t EBSetOutputFileCallback(
    const EBOutputFileCallback_t outputFileCallback);

/**
 * @brief GUI settings
 *
 * @param ipAddress to bind to, normally loopback but could be any address
 * @param configRoot directory containing configuration files
 * @param httpRoot directory containing HTTP top level
 * @param httpPort http server will attempt to open to, 0 will use the
 * automatically pick one
 * @param timeoutIdle in seconds to wait before exiting when no connections are
 * in progress (allow time for browser to load new pages)
 * @param callbackThreadPool is the number of threads in the pool to handle
 * callbacks
 */
struct EBGUISettings_t {
  char *   ipAddress = "127.0.0.1";
  char *   configRoot;
  char *   httpRoot;
  uint16_t httpPort           = 0;
  uint8_t  timeoutIdle        = 5;
  uint8_t  callbackThreadPool = 8;
};

/**
 * @brief Create a GUI using the settings then open the preferred then default
 * browser
 *
 * @param guiSettings to use to create GUI
 * @return EBError_t zero on success, non-zero on failure
 */
extern "C" EHBANANA_API EBError_t EBLaunch(const EBGUISettings_t guiSettings);

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
 * @return EBError_t zero on success, non-zero on failure
 */
extern "C" EHBANANA_API EBError_t EBDestroy();

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
 * @return EBError_t zero on success, non-zero on failure
 */
extern "C" EHBANANA_API EBError_t EBEnqueueOutput(
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
 * @param EBError_t errorCode to get the string for
 * @return const char * constant string
 */
extern "C" EHBANANA_API const char * EBErrorName(EBError_t errorCode);

#endif /* _EHBANANA_H_ */