#include "Ehbanana.h"

#include <iostream>
#include <memory>
#include <string>

#include "spdlog/sinks/rotating_file_sink.h"
#include "spdlog/sinks/stdout_sinks.h"
#include "spdlog/spdlog.h"

#include "Hash.h"
#include "Result.h"
#include "web/Server.h"

/**
 * @brief Entry point for the program
 * 
 * @param argc command line argument count
 * @param argv command line arguments
 * @return int error code
 */
int main(int argc, char * argv[]) {
  setupLogging(argc, argv);
  spdlog::info("Ehbanana Version {}.{}.{}", VERSION[0], VERSION[1], VERSION[2]);

  Web::Server       server;
  Results::Result_t result = Results::SUCCESS;

  result = server.initialize();
  if(!result){
    spdlog::error(result);
    return result.value;
  }

  server.start();
  spdlog::info("Press [ENTER] to shutdown");
  std::cin.ignore();
  server.stop();

  return result.value;
}

/**
 * @brief Initializes a rotating log, and (if DEBUG_ON) console output
 *
 * @param argc command line argument count
 * @param argv command line argument array
 */
void setupLogging(int argc, char * argv[]) {
  while ((argc--) > 0) {
    Hash_t hash = Hash::calculateHash(argv[argc]);
    if (hash == Hash::calculateHash("--debug") ||
        hash == Hash::calculateHash("-d"))
      DEBUG_ON = true;
  }
  try {
    std::vector<spdlog::sink_ptr> sinks;
    if (DEBUG_ON) {
      sinks.push_back(
          std::make_shared<spdlog::sinks::wincolor_stdout_sink_mt>());
    }
    sinks.push_back(std::make_shared<spdlog::sinks::rotating_file_sink_mt>(
        "logfile.log", 5 * 1024 * 1024, 3));
    std::shared_ptr<spdlog::logger> logger =
        std::make_shared<spdlog::logger>("", begin(sinks), end(sinks));
    spdlog::set_default_logger(logger);
    if (DEBUG_ON) {
      spdlog::set_level(spdlog::level::debug); // Set global log level to debug
      spdlog::debug("Debug is enabled");
    }
  } catch (const spdlog::spdlog_ex & e) {
    // TODO Replace with error message dialog
    std::cout << "Log initialization failed: " << e.what() << std::endl;
  }
}