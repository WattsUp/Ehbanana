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
  mimeTypes(configRoot + "/mime.types"),
  cacheControl(configRoot + "/cache.xml") {
  this->root = httpRoot;
}

/**
 * @brief Handle a request and populate a reply
 *
 * @param request to handle
 * @param reply to populate
 * @return Result error code
 */
Result RequestHandler::handle(const Request & request, Reply & reply) {
  switch (request.getMethod().get()) {
    case Hash::calculateHash("GET"):
      return handleGET(request, reply) + "Handling GET";
    case Hash::calculateHash("POST"):
      return handlePOST(request, reply) + "Handling POST";
    default:
      return ResultCode_t::UNKNOWN_HASH +
             ("Request method: " + request.getMethod().getString());
  }
}

/**
 * @brief Handle a GET request and populate a reply
 *
 * @param request to handle
 * @param reply to populate
 * @return Result error code
 */
Result RequestHandler::handleGET(const Request & request, Reply & reply) {
  std::string uri = request.getURI().getString();
  if (request.getQueries().empty())
    spdlog::info("{} GET URI: \"{}\"", request.getEndpointString(), uri);
  else {
    std::string buffer = "";
    for (HeaderHash_t query : request.getQueries()) {
      buffer += "\n    \"" + query.name.getString() + "\"=\"" +
                query.value.getString() + "\"";
    }
    spdlog::info("{} GET URI: \"{}\" Queries:{}", request.getEndpointString(),
        uri, buffer);
  }

  if (uri.compare(0, 3, "/~/") == 0) {
    return handleEBFile(request, reply);
  }

  // URI must be absolute
  if (uri.empty() || uri[0] != '/' || uri.find("..") != std::string::npos)
    return ResultCode_t::INVALID_DATA + ("URI is not absolute: " + uri);

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
    return ResultCode_t::OPEN_FAILED + (root + uri);
  }
  reply.addHeader("Content-Length", std::to_string(file->size()));
  reply.addHeader("Cache-Control", cacheControl.getCacheControl(uri));
  reply.setContent(file);

  return ResultCode_t::SUCCESS;
}

/**
 * @brief Handle a POST request and populate a reply
 *
 * @param request to handle
 * @param reply to populate
 * @return Result error code
 */
Result RequestHandler::handlePOST(const Request & request, Reply & reply) {
  if (request.getQueries().empty())
    spdlog::info("POST URI: \"{}\"", request.getURI().getString());
  else {
    std::string buffer = "";
    for (HeaderHash_t query : request.getQueries()) {
      buffer += "\n    \"" + query.name.getString() + "\"=\"" +
                query.value.getString() + "\"";
    }
    spdlog::info(
        "POST URI: \"{}\" Queries:{}", request.getURI().getString(), buffer);
  }

  reply.setStatus(HTTPStatus_t::NOT_IMPLEMENTED);

  return ResultCode_t::NOT_SUPPORTED + "handlePOST";
}

/**
 * @brief Handles special files for setting up ehbanana gui
 *
 * @param request to handle
 * @param reply to populate
 * @return Result error code
 */
Result RequestHandler::handleEBFile(const Request & request, Reply & reply) {
  switch (request.getURI().get()) {
    case Hash::calculateHash("/~/websocket"): {
      std::string port = std::to_string(guiPort);
      reply.appendContent(port);
      reply.addHeader("Content-Length", std::to_string(port.length()));
    } break;
    default:
      return ResultCode_t::UNKNOWN_HASH + "Handling EBFile";
  }
  return ResultCode_t::SUCCESS;
}

/**
 * @brief Set the port used by the GUI
 * Passed to the web browser at the file "/~/websocket" in handleEBFile
 *
 * @param port to set
 */
void RequestHandler::setGUIPort(uint16_t port) {
  guiPort = port;
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