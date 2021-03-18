#pragma once

#include <cvs/logger/configtypes.hpp>
#include <cvs/logger/cvsloggerexport.hpp>

#include <spdlog/logger.h>

#include <filesystem>
#include <iostream>

namespace cvs::logger {

class CVSLOGGER_EXPORT ILogger {
 public:
  virtual ~ILogger() = default;

  virtual std::string_view             name() const     = 0;
  virtual Level                        level() const    = 0;
  virtual const std::filesystem::path& path() const     = 0;
  virtual LogImage                     logImage() const = 0;

  virtual bool isEnabled(Level) const = 0;

  template <typename T>
  struct Strategy {
    using Type = T;
  };

  template <typename T>
  typename Strategy<T>::Type processArg(Level, const T& arg) {
    return arg;
  }

  template <typename FormatString, typename... Args>
  void log(Level lvl, const FormatString& fmt, const Args&... args) {
    auto arg_tuple = std::apply([&](auto... a) { return std::make_tuple(processArg(lvl, a)...); },
                                std::make_tuple(args...));
    std::apply([&](auto... a) { logger->log(convertLogLevel(lvl), fmt, a...); }, arg_tuple);
  }

 protected:
  ILogger(std::shared_ptr<spdlog::logger> ptr)
      : logger(std::move(ptr)) {}

  static spdlog::level::level_enum convertLogLevel(Level l) {
    switch (l) {
      case Level::trace: return spdlog::level::level_enum::trace;
      case Level::debug: return spdlog::level::level_enum::debug;
      case Level::info: return spdlog::level::level_enum::info;
      case Level::warn: return spdlog::level::level_enum::warn;
      case Level::err: return spdlog::level::level_enum::err;
      case Level::critical: return spdlog::level::level_enum::critical;
      case Level::off: return spdlog::level::level_enum::off;
    }

    return spdlog::level::level_enum::off;
  }

  static Level convertLogLevel(spdlog::level::level_enum l) {
    switch (l) {
      case spdlog::level::level_enum::trace: return Level::trace;
      case spdlog::level::level_enum::debug: return Level::debug;
      case spdlog::level::level_enum::info: return Level::info;
      case spdlog::level::level_enum::warn: return Level::warn;
      case spdlog::level::level_enum::err: return Level::err;
      case spdlog::level::level_enum::critical: return Level::critical;
      case spdlog::level::level_enum::off: return Level::off;
      default: return Level::off;
    }

    return Level::off;
  }

  // While there is no format implementation in std, it will be like this:
  std::shared_ptr<spdlog::logger> logger;
};

}  // namespace cvs::logger

#ifdef CVS_LOGGER_OPENCV_ENABLED

#include <atomic>
#include <opencv2/highgui/highgui.hpp>

namespace cvs::logger {

template <>
struct ILogger::Strategy<cv::Mat> {
  using Type = std::string;
};

template <>
ILogger::Strategy<cv::Mat>::Type ILogger::processArg<cv::Mat>(Level l, const cv::Mat& arg);

}  // namespace cvs::logger

#endif
