#include "RequestHandler.h"

namespace Web {

/**
 * @brief Construct a new Request Handler:: Request Handler object
 * 
 * @param root directory to serve http pages
 */
RequestHandler::RequestHandler(const std::string & root) {
  this->root = root;
}

} // namespace Web