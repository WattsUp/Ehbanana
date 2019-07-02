#ifndef _RESULT_H_
#define _RESULT_H_

#include <stdint.h>

typedef uint16_t EBResult_t;

// clang-format off
#define EBRESULT_SUCCESS                0x0000
#define EBRESULT_INVALID_FUNCTION       0x0001
#define EBRESULT_ACCESS_DENIED          0x0002
#define EBRESULT_NOT_ENOUGH_MEMORY      0x0003
#define EBRESULT_INVALID_DATA           0x0004
#define EBRESULT_DISK_FULL              0x0005
#define EBRESULT_BAD_COMMAND            0x0006
#define EBRESULT_CRC                    0x0007
#define EBRESULT_WRITE_FAULT            0x0008
#define EBRESULT_READ_FAULT             0x0009
#define EBRESULT_END_OF_FILE            0x000A
#define EBRESULT_NOT_SUPPORTED          0x000B
#define EBRESULT_FILE_EXISTS            0x000C
#define EBRESULT_CANNOT_MAKE            0x000D
#define EBRESULT_INVALID_PASSWORD       0x000E
#define EBRESULT_INVALID_PARAMETER      0x000F
#define EBRESULT_OPEN_FAILED            0x0010
#define EBRESULT_BUFFER_OVERFLOW        0x0011
#define EBRESULT_DIR_NOT_EMPTY          0x0012
#define EBRESULT_VERSION_NOT_SUPPORTED  0x0013
#define EBRESULT_INVALID_UTF8           0x0014
#define EBRESULT_UNKNOWN_HASH           0x0015
#define EBRESULT_UNKNOWN_REFERENCE      0x0016
#define EBRESULT_EXCEPTION_OCCURED      0x0017
#define EBRESULT_UNDEFINED_PARENT       0x0018
#define EBRESULT_INCOMPLETE_OPERATION   0x0019
#define EBRESULT_NO_OPERATION           0x001A
#define EBRESULT_TIMEOUT                0x001B
// clang-format on

/**
 * @brief Returns true if the result is an error
 * 
 */
#define EBRESULT_ERROR(result) (result != EBRESULT_SUCCESS)

#endif /* _RESULT_H_ */