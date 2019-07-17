#ifndef _RESULT_H_
#define _RESULT_H_

#include <stdint.h>

typedef uint16_t EBResult_t;

// clang-format off

#define EBRESULT_SUCCESS                (EBResult_t)0x0000
#define EBRESULT_INVALID_FUNCTION       (EBResult_t)0x0001
#define EBRESULT_ACCESS_DENIED          (EBResult_t)0x0002
#define EBRESULT_NOT_ENOUGH_MEMORY      (EBResult_t)0x0003
#define EBRESULT_INVALID_DATA           (EBResult_t)0x0004
#define EBRESULT_DISK_FULL              (EBResult_t)0x0005
#define EBRESULT_BAD_COMMAND            (EBResult_t)0x0006
#define EBRESULT_CRC                    (EBResult_t)0x0007
#define EBRESULT_WRITE_FAULT            (EBResult_t)0x0008
#define EBRESULT_READ_FAULT             (EBResult_t)0x0009
#define EBRESULT_END_OF_FILE            (EBResult_t)0x000A
#define EBRESULT_NOT_SUPPORTED          (EBResult_t)0x000B
#define EBRESULT_FILE_EXISTS            (EBResult_t)0x000C
#define EBRESULT_CANNOT_MAKE            (EBResult_t)0x000D
#define EBRESULT_INVALID_PASSWORD       (EBResult_t)0x000E
#define EBRESULT_INVALID_PARAMETER      (EBResult_t)0x000F
#define EBRESULT_OPEN_FAILED            (EBResult_t)0x0010
#define EBRESULT_BUFFER_OVERFLOW        (EBResult_t)0x0011
#define EBRESULT_DIR_NOT_EMPTY          (EBResult_t)0x0012
#define EBRESULT_VERSION_NOT_SUPPORTED  (EBResult_t)0x0013
#define EBRESULT_INVALID_UTF8           (EBResult_t)0x0014
#define EBRESULT_UNKNOWN_HASH           (EBResult_t)0x0015
#define EBRESULT_UNKNOWN_REFERENCE      (EBResult_t)0x0016
#define EBRESULT_EXCEPTION_OCCURED      (EBResult_t)0x0017
#define EBRESULT_UNDEFINED_PARENT       (EBResult_t)0x0018
#define EBRESULT_INCOMPLETE_OPERATION   (EBResult_t)0x0019
#define EBRESULT_NO_OPERATION           (EBResult_t)0x001A
#define EBRESULT_TIMEOUT                (EBResult_t)0x001B
#define EBRESULT_NO_SYSTEM_CALL         (EBResult_t)0x001C
#define EBRESULT_INVALID_STATE          (EBResult_t)0x001D

#define EBRESULT_ASIO_MAKE_ADDRESS      (EBResult_t)0x1000
#define EBRESULT_ASIO_OPEN_ACCEPTOR     (EBResult_t)0x1001
#define EBRESULT_ASIO_SET_OPTION        (EBResult_t)0x1002
#define EBRESULT_ASIO_BIND              (EBResult_t)0x1003
#define EBRESULT_ASIO_LISTEN            (EBResult_t)0x1004
#define EBRESULT_ASIO_WRITE_FAULT       (EBResult_t)0x1005
#define EBRESULT_ASIO_READ_FAULT        (EBResult_t)0x1006

/**
 * @brief Returns true if the result is an error
 *
 */
#define EBRESULT_ERROR(result) (result != EBRESULT_SUCCESS)


#endif /* _RESULT_H_ */