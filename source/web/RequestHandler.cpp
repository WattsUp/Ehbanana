#include "RequestHandler.h"

#include <spdlog/spdlog.h>

namespace Web {

/**
 * @brief Construct a new Request Handler:: Request Handler object
 *
 * @param httpRoot directory to serve http pages
 * @param configRoot containing the configuration files
 */
RequestHandler::RequestHandler(
    const std::string & httpRoot, const std::string & configRoot) :
  mimeTypes(configRoot + "/mime.types") {
  this->root = httpRoot;
}

/**
 * @brief Handle a request and populate a reply
 *
 * @param request to handle
 * @param reply to populate
 * @return EBResultMsg_t error code
 */
EBResultMsg_t RequestHandler::handle(const Request & request, Reply & reply) {
  reply.reset();
  switch (request.getMethod().hash) {
    case Hash::calculateHash("GET"):
      return handleGET(request, reply);
    case Hash::calculateHash("POST"):
      return handlePOST(request, reply);
    default:
      return EBResult::UNKNOWN_HASH +
             ("Request method: " + request.getMethod().string);
  }
}

/**
 * @brief Handle a GET request and populate a reply
 *
 * @param request to handle
 * @param reply to populate
 * @return EBResultMsg_t error code
 */
EBResultMsg_t RequestHandler::handleGET(
    const Request & request, Reply & reply) {
  std::string uri = request.getURI().string;
  if (request.getQueries().empty())
    spdlog::info("{} GET URI: \"{}\"", request.getEndpointString(), uri);
  else {
    std::string buffer = "";
    for (HeaderHash_t query : request.getQueries()) {
      buffer +=
          "\n    \"" + query.name.string + "\"=\"" + query.value.string + "\"";
    }
    spdlog::info("{} GET URI: \"{}\" Queries:{}", request.getEndpointString(),
        uri, buffer);
  }

  // URI must be absolute
  if (uri.empty() || uri[0] != '/' || uri.find("..") != std::string::npos)
    return EBResult::INVALID_DATA + ("URI is not absolute: " + uri);

  // Add index.html to folders
  if (uri[uri.size() - 1] == '/')
    uri += "index.html";

  // Determine the file extension.
  reply.addHeader("Content-Type", fileToType(uri));
  MemoryMapped * file =
      new MemoryMapped(root + uri, 0, MemoryMapped::SequentialScan);
  if (!file->isValid()) {
    file->close();
    delete file;
    return EBResult::OPEN_FAILED + (root + uri);
  }
  reply.addHeader("Content-Length", std::to_string(file->size()));
  reply.setContent(file);

  return EBResult::SUCCESS;
}

/**
 * @brief Handle a POST request and populate a reply
 *
 * @param request to handle
 * @param reply to populate
 * @return EBResultMsg_t error code
 */
EBResultMsg_t RequestHandler::handlePOST(
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

  reply.setStatus(HTTPStatus_t::NOT_IMPLEMENTED);

  return EBResult::NOT_SUPPORTED + "handlePOST";
}

/**
 * @brief Returns the MIME type of the file based on its extension
 *
 * @param file name to parse
 * @return std::string MIME type
 */
std::string RequestHandler::fileToType(const std::string & file) {
  std::string fileExtension;
  bool        dotPresent = false;
  for (char c : file) {
    if (c == '/') {
      fileExtension.erase();
      dotPresent = false;
    } else if (c == '.') {
      fileExtension = ".";
      dotPresent    = true;
    } else if (dotPresent)
      fileExtension += c;
  }
  return mimeTypes.getType(fileExtension);
}

} // namespace Web