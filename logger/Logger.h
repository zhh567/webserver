#ifndef WEBSERVER_LONGGER_LOGGER_H
#define WEBSERVER_LONGGER_LOGGER_H

#include <webserver/base/Buffer.h>

#include <functional>
#include <string>
#include <vector>

namespace webserver {

class Logger {
 public:
  enum class LogLevelEnum {
    DEBUG,
    INFO,
    WARN,
    ERROR,
    FATAL,
    NUM_LOG_LEVEL,
  };

  // static const char* LogLevelEnumString[LogLevelEnum::NUM_LOG_LEVEL] =
  //     [ "DEBUG", "INFO", "WARN", "ERROR", "FATAL" ];

  Logger(std::string filename, int line, std::string func, LogLevelEnum level);
  ~Logger();

  static LogLevelEnum get_log_level();
  static void set_log_level(LogLevelEnum log_level);
  static LogLevelEnum kLogLevel;

  using OutputFunc = std::function<void(const char* data, int len)>;
  using FlushFunc = std::function<void()>;

  static Logger::OutputFunc kOutputFunc;
  static Logger::FlushFunc kFlushFunc;

  static void set_output(OutputFunc);
  static void set_flush(FlushFunc);

  using self = Logger;

  self& operator<<(short);
  self& operator<<(unsigned short);
  self& operator<<(int);
  self& operator<<(unsigned int);
  self& operator<<(long);
  self& operator<<(unsigned long);
  self& operator<<(long long);
  self& operator<<(unsigned long long);
  self& operator<<(float);
  self& operator<<(double);
  self& operator<<(long double);
  self& operator<<(char);
  self& operator<<(const char*);
  self& operator<<(const unsigned char*);
  self& operator<<(const std::string&);
  self& operator<<(const void*);
  self& operator<<(bool);

 private:
  std::string basename_;
  int line_;
  std::string func_;
  Buffer buffer_;
  LogLevelEnum level_;
};  // class Logger
}  // namespace webserver

using namespace webserver;

#ifndef __func__
#ifndef __FUNCTION__
#define __func__ __FUNCTION__
#else
#define __func__ "[UNKNOW]"
#endif  // __FUNCTION__
#endif  // __func__

#define LOG_DEBUG                                             \
  if (Logger::get_log_level() <= Logger::LogLevelEnum::DEBUG) \
  Logger(__FILE__, __LINE__, __func__, Logger::LogLevelEnum::DEBUG)
#define LOG_INFO                                             \
  if (Logger::get_log_level() <= Logger::LogLevelEnum::INFO) \
  Logger(__FILE__, __LINE__, __func__, Logger::LogLevelEnum::INFO)
#define LOG_WARN \
  Logger(__FILE__, __LINE__, __func__, Logger::LogLevelEnum::WARN)
#define LOG_ERROR \
  Logger(__FILE__, __LINE__, __func__, Logger::LogLevelEnum::ERROR)
#define LOG_FATAL \
  Logger(__FILE__, __LINE__, __func__, Logger::LogLevelEnum::FATAL)

#endif  // WEBSERVER_LONGGER_LOGGER_H
