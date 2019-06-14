#include "Request.h"

namespace Web {

Request::Request(){}

Results::Result_t Request::parse(char * begin, char * end){
  return Results::NOT_SUPPORTED;
}

}