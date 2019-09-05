#include "EhbananaLog.h"

#include "cstdarg"

namespace Ehbanana {

/**
 * @brief Set the logger method used by the logger
 *
 * @param loggerMethod
 */
void Logger::set(EBLogger_t loggerMethod) {
  this->logger = loggerMethod;
}

/**
 * @brief Log a message to the logger method
 * Prints the message string to the destination stream, default: stdout
 *
 * @param EBLogLevel_t log level
 * @param char * string
 */
void Logger::log(const EBLogLevel_t level, const char * string) {
  if (logger != nullptr) {
    logger(level, string);
  } else {
    fputs(string, stdout);
    fputc('\n', stdout);
  }
}

/**
 * @brief Log a message with debug level
 *
 * @param string
 */
void debug(const char * string) {
  Logger::Instance()->log(EBLogLevel_t::EB_DEBUG, string);
}

/**
 * @brief Log a message with info level
 *
 * @param string
 */
void info(const char * string) {
  Logger::Instance()->log(EBLogLevel_t::EB_INFO, string);
}

/**
 * @brief Log a message with warning level
 *
 * @param string
 */
void warn(const char * string) {
  Logger::Instance()->log(EBLogLevel_t::EB_WARNING, string);
}

/**
 * @brief Log a message with error level
 *
 * @param string
 */
void error(const char * string) {
  Logger::Instance()->log(EBLogLevel_t::EB_ERROR, string);
}

/**
 * @brief Log a message with critical level
 *
 * @param string
 */
void crit(const char * string) {
  Logger::Instance()->log(EBLogLevel_t::EB_CRITICAL, string);
}

/**
 * @brief Log a message using format specifier
 *
 * @param level to log at
 * @param format string
 * @param ... args
 */
void log(EBLogLevel_t level, const char * format, ...) {
  int     bufLen = 128;
  char *  buf    = new char[bufLen];
  va_list args;
  va_start(args, format);
  int n = vsnprintf(buf, bufLen, format, args);
  if (n > bufLen) {
    delete buf;
    bufLen = n + 1;
    buf    = new char[bufLen];
    n      = vsnprintf(buf, bufLen, format, args);
    if (n > bufLen) {
      crit("Could not print format string");
      va_end(args);
      delete buf;
      return;
    }
  }
  va_end(args);
  Logger::Instance()->log(level, buf);
  delete buf;
}

} // namespace Ehbanana