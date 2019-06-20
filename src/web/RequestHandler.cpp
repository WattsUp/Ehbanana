#include "RequestHandler.h"

#include "spdlog/spdlog.h"

namespace Web {

/**
 * @brief Construct a new Request Handler:: Request Handler object
 *
 * @param root directory to serve http pages
 */
RequestHandler::RequestHandler(const std::string & root) {
  this->root = root;
}

/**
 * @brief Handle a request and populate a reply
 *
 * @param request to handle
 * @param reply to populate
 * @return Results::Result_t error code
 */
Results::Result_t RequestHandler::handle(
    const Request & request, Reply & reply) {
  reply.reset();
  switch (request.getMethod().hash) {
    case Hash::calculateHash("GET"):
      return handleGET(request, reply);
    case Hash::calculateHash("POST"):
      return handlePOST(request, reply);
    default:
      return Hash::unknownHash("HTTP request method", request.getMethod());
  }
}

/**
 * @brief Handle a GET request and populate a reply
 *
 * @param request to handle
 * @param reply to populate
 * @return Results::Result_t error code
 */
Results::Result_t RequestHandler::handleGET(
    const Request & request, Reply & reply) {
  if (request.getQueries().empty())
    spdlog::info("GET URI: \"{}\"", request.getURI().string);
  else {
    std::string buffer = "";
    for (HeaderHash_t query : request.getQueries()) {
      buffer +=
          "\n    \"" + query.name.string + "\"=\"" + query.value.string + "\"";
    }
    spdlog::info("GET URI: \"{}\" Queries:{}", request.getURI().string, buffer);
  }

  return Results::NOT_SUPPORTED;
}

/**
 * @brief Handle a POST request and populate a reply
 *
 * @param request to handle
 * @param reply to populate
 * @return Results::Result_t error code
 */
Results::Result_t RequestHandler::handlePOST(
    const Request & request, Reply & reply) {
  if (request.getQueries().empty())
    spdlog::info("POST URI: \"{}\"", request.getURI().string);
  else {
    std::string buffer = "";
    for (HeaderHash_t query : request.getQueries()) {
      buffer +=
          "\n    \"" + query.name.string + "\"=\"" + query.value.string + "\"";
    }
    spdlog::info(
        "POST URI: \"{}\" Queries:{}", request.getURI().string, buffer);
  }

  return Results::NOT_SUPPORTED;
}

} // namespace Web