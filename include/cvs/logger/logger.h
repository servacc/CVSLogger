#pragma once

#include <cvs/logger/cvslogger_export.h>

#include <spdlog/spdlog.h>

#include <filesystem>
#include <memory>

#ifdef CVS_LOGGER_OPENCV_ENABLED
#include <opencv2/core/mat.hpp>
#endif

namespace cvs::logger {

class LoggerName : public std::string_view {
 public:
  constexpr explicit LoggerName(const std::string_view& other) noexcept
      : std::string_view(other) {}
  constexpr explicit LoggerName(const char* s, size_type count)
      : std::string_view(s, count) {}
  constexpr explicit LoggerName(const char* s)
      : std::string_view(s) {}
};

class LoggerPattern : public std::string_view {
 public:
  constexpr explicit LoggerPattern(const std::string_view& other) noexcept
      : std::string_view(other) {}
  constexpr explicit LoggerPattern(const char* s, size_type count)
      : std::string_view(s, count) {}
  constexpr explicit LoggerPattern(const char* s)
      : std::string_view(s) {}
};

namespace literals {

template <typename CharT, CharT... String>
constexpr LoggerName operator"" _name() {
  constexpr const char value[sizeof...(String) + 1] = {String...};
  constexpr int        size                         = sizeof...(String);
  return LoggerName(value, size);
}

template <typename CharT, CharT... String>
constexpr LoggerPattern operator"" _pattern() {
  constexpr const char value[sizeof...(String) + 1] = {String...};
  constexpr int        size                         = sizeof...(String);
  return LoggerPattern(value, size);
}

}  // namespace literals

enum class Level {
  trace    = 0,
  debug    = 1,
  info     = 2,
  warn     = 3,
  err      = 4,
  critical = 5,
  off      = 6,
};

enum class TimeType { local = 0, utc };

enum class LogImage { disable = 0, enable };

using LoggerPtr = std::shared_ptr<class Logger>;

class CVSLOGGER_EXPORT Logger {
 public:
  static LoggerPtr getLogger(std::string_view);

  static void configure(const LoggerPtr&) {}
  static void configure() {}

  template <typename... Args>
  static void configure(LoggerName name, Args... args) {
    auto logger = getLogger(name);
    configure(logger, args...);
  }

  template <typename... Args>
  static void configure(const LoggerPtr& logger, Level level, Args... args) {
    logger->setLevel(level);
    configure(logger, args...);
  }

  template <typename... Args>
  static void configure(const LoggerPtr& logger, LoggerPattern pattern, Args... args) {
    logger->setPattern(std::string(pattern));
    configure(logger, args...);
  }

  template <typename... Args>
  static void configure(const LoggerPtr&   logger,
                        const std::string& pattern,
                        TimeType           time_type,
                        Args... args) {
    logger->setPattern(pattern, time_type);
    configure(logger, args...);
  }

  template <typename... Args>
  static void configure(Level level, Args... args) {
    Logger::setLevelGlobal(level);
    configure(args...);
  }

  template <typename... Args>
  static void configure(LoggerPattern pattern, Args... args) {
    Logger::setPatternGlobal(std::string(pattern));
    configure(args...);
  }

  template <typename... Args>
  static void configure(LoggerPattern pattern, TimeType time_type, Args... args) {
    Logger::setPatternGlobal(std::string(pattern), time_type);
    configure(args...);
  }

  Logger(std::shared_ptr<spdlog::logger> logger);
  ~Logger();

  Logger(Logger&&) = delete;
  Logger(Logger&)  = delete;

  Logger& operator=(Logger&&) = delete;
  Logger& operator=(Logger&) = delete;

  void setLevel(Level);
  void setPattern(std::string, TimeType = TimeType::local);

  bool isEnabled(Level) const;

  template <typename... Args>
  void log(Level level, const Args&... args) {
    logger->log(convertLogLevel(level), args...);
  }

  template <typename... Args>
  static void logGlobal(Level l, const Args&... args) {
    initDefaultLogger();
    switch (l) {
      case Level::trace: spdlog::trace(args...); break;
      case Level::debug: spdlog::debug(args...); break;
      case Level::info: spdlog::info(args...); break;
      case Level::warn: spdlog::warn(args...); break;
      case Level::err: spdlog::error(args...); break;
      case Level::critical: spdlog::critical(args...); break;
      default: break;
    }
  }

  static void setLevelGlobal(Level);
  static void setPatternGlobal(std::string, TimeType = {});

  static spdlog::level::level_enum convertLogLevel(Level);
  static spdlog::pattern_time_type convertTimeType(TimeType);

 protected:
  static void                            initDefaultLogger();
  static std::shared_ptr<spdlog::logger> createLogger(std::string name = {});

 private:
  std::shared_ptr<spdlog::logger> logger;

  static spdlog::level::level_enum default_level;

#ifdef CVS_LOGGER_OPENCV_ENABLED
 public:
  static void setImgDir(std::filesystem::path);

  void setLogImage(LogImage);
  void setImageExtension(const std::string&);

  LogImage logImage() const;

  void log(Level level, const char* name, const cv::Mat& img);
  void log(Level level, const std::string& name, const cv::Mat& img);

 private:
  std::unordered_map<std::string, std::size_t> counters;

  LogImage    image_log = LogImage::disable;
  std::string image_ext = "png";

  static std::filesystem::path img_dir;
#endif
};

}  // namespace cvs::logger

#define LOG_TRACE(CH, ...)                            \
  if (CH && CH->isEnabled(cvs::logger::Level::trace)) \
  CH->log(cvs::logger::Level::trace, __VA_ARGS__)

#define LOG_DEBUG(CH, ...)                            \
  if (CH && CH->isEnabled(cvs::logger::Level::debug)) \
  CH->log(cvs::logger::Level::debug, __VA_ARGS__)

#define LOG_INFO(CH, ...)                            \
  if (CH && CH->isEnabled(cvs::logger::Level::info)) \
  CH->log(cvs::logger::Level::info, __VA_ARGS__)

#define LOG_WARN(CH, ...)                            \
  if (CH && CH->isEnabled(cvs::logger::Level::warn)) \
  CH->log(cvs::logger::Level::warn, __VA_ARGS__)

#define LOG_ERROR(CH, ...)                          \
  if (CH && CH->isEnabled(cvs::logger::Level::err)) \
  CH->log(cvs::logger::Level::err, __VA_ARGS__)

#define LOG_CRITICAL(CH, ...)                            \
  if (CH && CH->isEnabled(cvs::logger::Level::critical)) \
  CH->log(cvs::logger::Level::critical, __VA_ARGS__)

#define LOG_GLOB_TRACE(args...) cvs::logger::Logger::logGlobal(cvs::logger::Level::trace, args)
#define LOG_GLOB_DEBUG(args...) cvs::logger::Logger::logGlobal(cvs::logger::Level::debug, args)
#define LOG_GLOB_INFO(args...) cvs::logger::Logger::logGlobal(cvs::logger::Level::info, args)
#define LOG_GLOB_WARN(args...) cvs::logger::Logger::logGlobal(cvs::logger::Level::warn, args)
#define LOG_GLOB_ERROR(args...) cvs::logger::Logger::logGlobal(cvs::logger::Level::err, args)
#define LOG_GLOB_CRITICAL(args...) \
  cvs::logger::Logger::logGlobal(cvs::logger::Level::critical, args)
