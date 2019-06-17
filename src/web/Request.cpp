#include "Request.h"

namespace Web {

/**
 * @brief Construct a new Request:: Request object
 * 
 */
Request::Request(){}

/**
 * @brief Parse a string and add its contents to the request
 * 
 * The string may be a portion of the entire request
 * 
 * @param begin character pointer
 * @param end character pointer
 * @return Results::Result_t error code
 */
Results::Result_t Request::parse(char * begin, char * end){
  return Results::NOT_SUPPORTED;
}

}