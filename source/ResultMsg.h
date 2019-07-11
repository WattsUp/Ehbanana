#ifndef _RESULT_MSG_H_
#define _RESULT_MSG_H_

#include "Result.h"

#include <iostream>
#include <spdlog/fmt/ostr.h>
#include <stdint.h>
#include <string>

struct EBResultMsg_t {
  EBResult_t  value;
  std::string message;
};

namespace EBResult {

static const EBResultMsg_t SUCCESS               = {EBRESULT_SUCCESS,               "No error encountered"};
static const EBResultMsg_t INVALID_FUNCTION      = {EBRESULT_INVALID_FUNCTION,      "Incorrect function called"};
static const EBResultMsg_t ACCESS_DENIED         = {EBRESULT_ACCESS_DENIED,         "Access is denied"};
static const EBResultMsg_t NOT_ENOUGH_MEMORY     = {EBRESULT_NOT_ENOUGH_MEMORY,     "Not enough memory to complete the command"};
static const EBResultMsg_t INVALID_DATA          = {EBRESULT_INVALID_DATA,          "The data is invalid"};
static const EBResultMsg_t DISK_FULL             = {EBRESULT_DISK_FULL,             "Not enough disk space to complete the command"};
static const EBResultMsg_t BAD_COMMAND           = {EBRESULT_BAD_COMMAND,           "The command is not recognized"};
static const EBResultMsg_t CRC                   = {EBRESULT_CRC,                   "Data error (cyclic redundency check)"};
static const EBResultMsg_t WRITE_FAULT           = {EBRESULT_WRITE_FAULT,           "Cannot write to the specified device"};
static const EBResultMsg_t READ_FAULT            = {EBRESULT_READ_FAULT,            "Cannot read from the specified device"};
static const EBResultMsg_t END_OF_FILE           = {EBRESULT_END_OF_FILE,           "Reached EOF before completing the command"};
static const EBResultMsg_t NOT_SUPPORTED         = {EBRESULT_NOT_SUPPORTED,         "The command is not supported"};
static const EBResultMsg_t FILE_EXISTS           = {EBRESULT_FILE_EXISTS,           "The file already exists"};
static const EBResultMsg_t CANNOT_MAKE           = {EBRESULT_CANNOT_MAKE,           "Cannot create the specified directory or file"};
static const EBResultMsg_t INVALID_PASSWORD      = {EBRESULT_INVALID_PASSWORD,      "The specified password is not correct"};
static const EBResultMsg_t INVALID_PARAMETER     = {EBRESULT_INVALID_PARAMETER,     "The specified parameter is not correct"};
static const EBResultMsg_t OPEN_FAILED           = {EBRESULT_OPEN_FAILED,           "Could not open the specified file or device"};
static const EBResultMsg_t BUFFER_OVERFLOW       = {EBRESULT_BUFFER_OVERFLOW,       "The buffer exceeded its size"};
static const EBResultMsg_t DIR_NOT_EMPTY         = {EBRESULT_DIR_NOT_EMPTY,         "The directory is not empty"};
static const EBResultMsg_t VERSION_NOT_SUPPORTED = {EBRESULT_VERSION_NOT_SUPPORTED, "The version is not compatible with the command"};
static const EBResultMsg_t INVALID_UTF8          = {EBRESULT_INVALID_UTF8,          "The character does not follow the UTF8 standard"};
static const EBResultMsg_t UNKNOWN_HASH          = {EBRESULT_UNKNOWN_HASH,          "The hash is not recognized in the list of known hashes"};
static const EBResultMsg_t UNKNOWN_REFERENCE     = {EBRESULT_UNKNOWN_REFERENCE,     "The reference is to an undefined object"};
static const EBResultMsg_t EXCEPTION_OCCURED     = {EBRESULT_EXCEPTION_OCCURED,     "The command encountered an exception"};
static const EBResultMsg_t UNDEFINED_PARENT      = {EBRESULT_UNDEFINED_PARENT,      "The parent of the object is undefined"};
static const EBResultMsg_t INCOMPLETE_OPERATION  = {EBRESULT_INCOMPLETE_OPERATION,  "The operation has not completed"};
static const EBResultMsg_t NO_OPERATION          = {EBRESULT_NO_OPERATION,          "No operation was performed"};
static const EBResultMsg_t TIMEOUT               = {EBRESULT_TIMEOUT,               "The operation did not complete before a timeout expired"};
static const EBResultMsg_t NO_SYSTEM_CALL        = {EBRESULT_NO_SYSTEM_CALL,        "System call is not available to use"};
static const EBResultMsg_t INVALID_STATE         = {EBRESULT_INVALID_STATE,         "The current state is not valid"};

static const EBResultMsg_t ASIO_MAKE_ADDRESS     = {EBRESULT_ASIO_MAKE_ADDRESS,     "ASIO failed to make an address"};
static const EBResultMsg_t ASIO_OPEN_ACCEPTOR    = {EBRESULT_ASIO_OPEN_ACCEPTOR,    "ASIO failed to open an acceptor"};
static const EBResultMsg_t ASIO_SET_OPTION       = {EBRESULT_ASIO_SET_OPTION,       "ASIO failed to set an option"};
static const EBResultMsg_t ASIO_BIND             = {EBRESULT_ASIO_BIND,             "ASIO failed to bind to an address"};
static const EBResultMsg_t ASIO_LISTEN           = {EBRESULT_ASIO_LISTEN,           "ASIO failed to be put in listen mode"};
static const EBResultMsg_t ASIO_WRITE_FAULT      = {EBRESULT_ASIO_WRITE_FAULT,      "ASIO failed to write to a socket"};
static const EBResultMsg_t ASIO_READ_FAULT       = {EBRESULT_ASIO_READ_FAULT,       "ASIO failed to read from a socket"};

// clang-format on

} // namespace EBResult

std::ostream & operator<<(std::ostream & stream, const EBResultMsg_t & result);
bool operator!=(const EBResultMsg_t & left, const EBResultMsg_t & right);
bool operator==(const EBResultMsg_t & left, const EBResultMsg_t & right);
bool operator!(const EBResultMsg_t & result);
EBResultMsg_t operator+(const EBResultMsg_t & left, const std::string & right);
EBResultMsg_t operator+(const EBResultMsg_t & left, const char * right);

#endif /* _RESULT_MSG_H_ */