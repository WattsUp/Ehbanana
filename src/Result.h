#ifndef _RESULT_H_
#define _RESULT_H_

#include <iostream>
#include <stdint.h>
#include <string>

#include "spdlog/fmt/ostr.h"

namespace Results {

typedef struct Result {
  uint16_t    value;
  std::string message;
} Result_t;

// clang-format off
static const Result_t SUCCESS               = {0x0000, "No error encountered"};
static const Result_t INVALID_FUNCTION      = {0x0001, "Incorrect function called"};
static const Result_t ACCESS_DENIED         = {0x0002, "Access is denied"};
static const Result_t NOT_ENOUGH_MEMORY     = {0x0003, "Not enough memory to complete the command"};
static const Result_t INVALID_DATA          = {0x0004, "The data is invalid"};
static const Result_t DISK_FULL             = {0x0005, "Not enough disk space to complete the command"};
static const Result_t BAD_COMMAND           = {0x0006, "The command is not recognized"};
static const Result_t CRC                   = {0x0007, "Data error (cyclic redundency check)"};
static const Result_t WRITE_FAULT           = {0x0008, "Cannot write to the specified device"};
static const Result_t READ_FAULT            = {0x0009, "Cannot read from the specified device"};
static const Result_t END_OF_FILE           = {0x000A, "Reached EOF before completing the command"};
static const Result_t NOT_SUPPORTED         = {0x000B, "The command is not supported"};
static const Result_t FILE_EXISTS           = {0x000C, "The file already exists"};
static const Result_t CANNOT_MAKE           = {0x000D, "Cannot create the specified directory or file"};
static const Result_t INVALID_PASSWORD      = {0x000E, "The specified password is not correct"};
static const Result_t INVALID_PARAMETER     = {0x000F, "The specified parameter is not correct"};
static const Result_t OPEN_FAILED           = {0x0010, "Could not open the specified file or device"};
static const Result_t BUFFER_OVERFLOW       = {0x0011, "The buffer exceeded its size"};
static const Result_t DIR_NOT_EMPTY         = {0x0012, "The directory is not empty"};
static const Result_t VERSION_NOT_SUPPORTED = {0x0013, "The version is not compatible with the command"};
static const Result_t INVALID_UTF8          = {0x0014, "The character does not follow the UTF8 standard"};
static const Result_t UNKNOWN_HASH          = {0x0015, "The hash is not recognized in the list of known hashes"};
static const Result_t UNKNOWN_REFERENCE     = {0x0016, "The reference is to an undefined object"};
static const Result_t EXCEPTION_OCCURED     = {0x0017, "The command encountered an exception"};
static const Result_t UNDEFINED_PARENT      = {0x0018, "The parent of the object is undefined"};
static const Result_t INCOMPLETE_OPERATION  = {0x0019, "The operation has not completed yet"};
static const Result_t NO_OPERATION          = {0x001A, "The operation did not execute"};
static const Result_t TIMEOUT               = {0x001B, "The operation did not finish before the timeout"};
// clang-format on
} // namespace Results

std::ostream & operator<<(
    std::ostream & stream, const Results::Result_t & result);
bool operator!=(
    const Results::Result_t & left, const Results::Result_t & right);
bool operator==(
    const Results::Result_t & left, const Results::Result_t & right);
bool              operator!(const Results::Result_t & result);
Results::Result_t operator+(const Results::Result_t & left, std::string right);

#endif /* _RESULT_H_ */