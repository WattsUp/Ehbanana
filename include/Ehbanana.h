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
  SUCCESS               = 0,
  EXCEPTION_OCCURRED    = 1,
  INITIALIZATION_FAILED = 2,
  START_FAILED          = 3,
  PROCESS_START         = 4,
  NOT_SUPPORTED         = 5,
  NO_SERVER_CREATED     = 6,
  END_OF_FILE           = 7,
  BUFFER_EMPTY          = 8,
  BUFFER_FULL           = 9,
  NULL_ARGUMENT         = 10,
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

typedef void * EBStream_t;

/**
 * @brief Read a byte from a stream
 *
 * @param stream to read from
 * @param buf to return
 * @return EBError_t zero on success, non-zero on failure
 */
extern "C" EHBANANA_API EBError_t EBStreamRead(
    EBStream_t stream, uint8_t * buf);

/**
 * @brief Read a block from a stream
 *
 * @param stream to read from
 * @param buf to copy into
 * @param length of the buffer, and number of bytes actually read upon return
 * @return EBError_t zero on success, non-zero on failure
 */
extern "C" EHBANANA_API EBError_t EBStreamReadBlock(
    EBStream_t stream, uint8_t * buf, size_t * length);

/**
 * @brief Write a byte to a stream
 *
 * @param stream to write to
 * @param buf to write
 * @return EBError_t zero on success, non-zero on failure
 */
extern "C" EHBANANA_API EBError_t EBStreamWrite(
    EBStream_t stream, const uint8_t buf);

/**
 * @brief Write a block to a stream
 *
 * @param stream to write to
 * @param buf to write
 * @param length of the buffer
 * @return EBError_t zero on success, non-zero on failure
 */
extern "C" EHBANANA_API EBError_t EBStreamWriteBlock(
    EBStream_t stream, const uint8_t * buf, size_t length);

/**
 * @brief Set the complete state of the stream to true
 * Called once the file has been fully written to the stream
 *
 */
extern "C" EHBANANA_API EBError_t EBStreamFinish(EBStream_t stream);

/**
 * @brief Process input file from the GUI
 *
 * @param id of the triggering element
 * @param value of the triggering element, usually the filename
 * @param file transferred from the GUI
 */
typedef void(__stdcall * EBInputFileCallback_t)(
    const char * id, const char * value, EBStream_t file);

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
struct EBQuery_t {
  const char *      name  = nullptr;
  const char *      value = nullptr;
  const EBQuery_t * next  = nullptr;
};

/**
 * @brief Process not found URI from the GUI
 * Called first if 404 error occurs
 * Return file extension if URI was properly handled and done writing
 * Return nullptr if the file is not to be found and to send a 404 error
 *
 * @param uri of the file being downloaded
 * @param queries following the uri
 * @param file to send to the GUI, set the complete flag once done writing
 * @return const char * file extension to look up mime type
 */
typedef const char *(__stdcall * EB404Callback_t)(
    const char * uri, const EBQuery_t * queries, EBStream_t file);

/**
 * @brief Set the callback to output files not found in the HTTP directory
 *
 * @param callback404 function
 * @return EBError_t zero on success, non-zero on failure
 */
extern "C" EHBANANA_API EBError_t EBSet404Callback(
    const EB404Callback_t callback404);

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
 * @brief Create a GUI using the settings
 *
 * @param guiSettings to use to create GUI
 * @return EBError_t zero on success, non-zero on failure
 */
extern "C" EHBANANA_API EBError_t EBCreate(const EBGUISettings_t guiSettings);

/**
 * @brief Start the webserver and open the preferred default
 * browser
 *
 * @return EBError_t zero on success, non-zero on failure
 */
extern "C" EHBANANA_API EBError_t EBLaunch();

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

typedef void * EBOutput_t;

/**
 * @brief Create an output object, server owns its memory and will automatically
 * dispose of it
 *
 * @param uri of the target page, nullptr will broadcast to all pages
 * @param output object, nullptr if failed
 * @return EBError_t zero on success, non-zero on failure
 */
extern "C" EHBANANA_API EBError_t EBCreateOutput(
    const char * uri, EBOutput_t * output);

// Enum of output types
enum class EBValueType_t : uint8_t {
  UINT8,
  INT8,
  UINT16,
  INT16,
  UINT32,
  INT32,
  UINT64,
  INT64,
  FLOAT,
  DOUBLE,
  CSTRING
};

/**
 * @brief Add a property/value pair to an output object, value will be deep
 * copied
 *
 * @param output object
 * @param id of the target element
 * @param property of element
 * @param type of value
 * @param value of property
 * @return EBError_t zero on success, non-zero on failure
 */
extern "C" EHBANANA_API EBError_t EBAddOutput(const EBOutput_t output,
    const char * id, const char * property, const EBValueType_t type,
    const void * value);

#include "ehbanana/AddOutput.h" // Type overrides

/**
 * @brief Enqueue an output object
 * Do not access the output object once enqueued, it is deleted
 *
 * @param output element to enqueue, server will delete memory once sent out
 * @return EBError_t zero on success, non-zero on failure
 */
extern "C" EHBANANA_API EBError_t EBEnqueueOutput(const EBOutput_t output);

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