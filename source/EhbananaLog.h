#ifndef _EHBANANA_LOG_H_
#define _EHBANANA_LOG_H_

#include "Ehbanana.h"

#include <string>

namespace Ehbanana {

class Logger {
public:
  Logger(const Logger &) = delete;
  Logger & operator=(const Logger &) = delete;

  /**
   * @brief Get the singleton instance
   *
   * @return Logger*
   */
  static Logger * Instance() {
    static Logger instance;
    return &instance;
  }

  void set(EBLogger_t loggerMethod);

  void log(const EBLogLevel_t level, const char * string);

private:
  /**
   * @brief Construct a new Logger object
   *
   */
  Logger() {}

  EBLogger_t logger = nullptr;
};

void debug(const char * string);
void info(const char * string);
void warn(const char * string);
void error(const char * string);
void crit(const char * string);

void log(EBLogLevel_t level, const char * format, ...);
// TODO make debug, info, etc take in format strings like this

/**
 * @brief Log a message with debug level
 *
 * @param string
 */
inline void debug(const std::string & string) {
  debug(string.c_str());
}

/**
 * @brief Log a message with info level
 *
 * @param string
 */
inline void info(const std::string & string) {
  info(string.c_str());
}

/**
 * @brief Log a message with warn level
 *
 * @param string
 */
inline void warn(const std::string & string) {
  warn(string.c_str());
}

/**
 * @brief Log a message with error level
 *
 * @param string
 */
inline void error(const std::string & string) {
  error(string.c_str());
}

/**
 * @brief Log a message with critical level
 *
 * @param string
 */
inline void crit(const std::string & string) {
  crit(string.c_str());
}

} // namespace Ehbanana

#endif /* _EHBANANA_LOG_H_ */