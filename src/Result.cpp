#include "Result.h"

/**
 * @brief Print a result to the stream with its code and message
 *
 * @param stream to write to
 * @param result object
 * @return std::ostream& stream passed in
 */
std::ostream & operator<<(
    std::ostream & stream, const Results::Result_t & result) {
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
bool operator!=(
    const Results::Result_t & left, const Results::Result_t & right) {
  return left.value != right.value;
}

/**
 * @brief Equal to comparision operator compares result values
 *
 * @param left result
 * @param right result
 * @return left.value == right.value
 */
bool operator==(
    const Results::Result_t & left, const Results::Result_t & right) {
  return left.value == right.value;
}

/**
 * @brief Add a message to an existing result
 *
 * @param left result
 * @param right message to append
 * @return Results::Result_t combined result
 */
Results::Result_t operator+(const Results::Result_t & left, std::string right) {
  return {left.value, left.message + "\n  Details: " + right};
}