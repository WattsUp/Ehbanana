#include "HTTPStatus.h"

/**
 * @brief Print a status to the stream with its code and message
 *
 * @param stream to write to
 * @param status object
 * @return std::ostream& stream passed in
 */
std::ostream & operator<<(
    std::ostream & stream, const Web::HTTPStatus::Status_t & status) {
  return stream << status.value << " " << status.message;
}