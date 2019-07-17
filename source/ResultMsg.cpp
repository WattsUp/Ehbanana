#include "ResultMsg.h"

/**
 * @brief Print a result to the stream with its code and message
 *
 * @param stream to write to
 * @param result object
 * @return std::ostream& stream passed in
 */
std::ostream & operator<<(std::ostream & stream, const EBResultMsg_t & result) {
  char buf[16];
  snprintf(buf, sizeof(buf), "0x%04X: ", result.value);
  stream << buf << result.message;
  return stream;
}

/**
 * @brief Not equal to comparision operator compares result values
 *
 * @param left result
 * @param right result
 * @return left.value != right.value
 */
bool operator!=(const EBResultMsg_t & left, const EBResultMsg_t & right) {
  return left.value != right.value;
}

/**
 * @brief Equal to comparision operator compares result values
 *
 * @param left result
 * @param right result
 * @return left.value == right.value
 */
bool operator==(const EBResultMsg_t & left, const EBResultMsg_t & right) {
  return left.value == right.value;
}

/**
 * @brief Not operator compares result value to SUCCESS
 *
 * @param result
 * @return result.value != EBRESULT_SUCCESS
 */
bool operator!(const EBResultMsg_t & result) {
  return result.value != EBRESULT_SUCCESS;
}

/**
 * @brief Add a message to an existing result
 *
 * @param left result
 * @param right message to append
 * @return EBResultMsg_t combined result
 */
EBResultMsg_t operator+(const EBResultMsg_t & left, const std::string & right) {
  return {left.value, left.message + "\n  Details: " + right};
}

/**
 * @brief Add a message to an existing result
 *
 * @param left result
 * @param right message to append
 * @return EBResultMsg_t combined result
 */
EBResultMsg_t operator+(const EBResultMsg_t & left, const char * right) {
  return {left.value, left.message + "\n  Details: " + right};
}