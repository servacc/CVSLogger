#pragma once

#include <cvs/logger/ilogger.hpp>
#include <cvs/logger/loggerfactory.hpp>

#include <spdlog/spdlog.h>
#include <cvs/common/config.hpp>

#ifdef CVS_LOGGER_OPENCV_ENABLED
#include <map>
#include <opencv2/core/mat.hpp>
#endif

namespace cvs::logger {

using Logger1Ptr = std::shared_ptr<spdlog::logger>;

void initDefaultLogger(std::optional<cvs::common::Config>);
void initLoggers(std::optional<cvs::common::Config> = std::nullopt);
void registerLogger();

template <typename T>
struct ProcessArg {
  static auto exec(Logger1Ptr&, spdlog::level::level_enum, T&& arg) { return std::forward<T>(arg); }
};

#ifdef CVS_LOGGER_OPENCV_ENABLED

template <>
struct ProcessArg<cv::Mat> {
  static std::string exec(Logger1Ptr&, spdlog::level::level_enum, const cv::Mat& arg);

  struct LoggerInfo {
    std::size_t                              counter = 0;
    std::optional<spdlog::level::level_enum> lvl;
    std::optional<std::filesystem::path>     path;
  };

  static std::map<std::string, LoggerInfo> save_info;

  static spdlog::level::level_enum default_save;
  static std::filesystem::path     default_path;
};

#endif

template <typename... Args>
void logHelper(Logger1Ptr&               logger,
               spdlog::level::level_enum lvl,
               spdlog::string_view_t     fmt,
               Args&&... args) {
  logger->log(lvl, fmt,
              ProcessArg<std::remove_cvref_t<Args>>(logger, lvl, std::forward<Args>(args))...);
}

}  // namespace cvs::logger

#define LOG_TRACE(CH, ...)                        \
  if (CH && CH->should_log(spdlog::level::trace)) \
  logHelper(CH, spdlog::level::trace, __VA_ARGS__)
#define LOG_DEBUG(CH, ...)                        \
  if (CH && CH->should_log(spdlog::level::debug)) \
  logHelper(CH, spdlog::level::debug, __VA_ARGS__)
#define LOG_INFO(CH, ...)                        \
  if (CH && CH->should_log(spdlog::level::info)) \
  logHelper(CH, spdlog::level::info, __VA_ARGS__)
#define LOG_WARN(CH, ...)                        \
  if (CH && CH->should_log(spdlog::level::warn)) \
  logHelper(CH, spdlog::level::warn, __VA_ARGS__)
#define LOG_ERROR(CH, ...)                      \
  if (CH && CH->should_log(spdlog::level::err)) \
  logHelper(CH, spdlog::level::err, __VA_ARGS__)
#define LOG_CRITICAL(CH, ...)                        \
  if (CH && CH->should_log(spdlog::level::critical)) \
  logHelper(CH, spdlog::level::critical, __VA_ARGS__)

#define LOG_GLOB_TRACE(args...)                                                         \
  if (auto logger = spdlog::default_logger(); logger->should_log(spdlog::level::trace)) \
  logHelper(logger, spdlog::level::trace, args)
#define LOG_GLOB_DEBUG(args...)                                                         \
  if (auto logger = spdlog::default_logger(); logger->should_log(spdlog::level::debug)) \
  logHelper(logger, spdlog::level::debug, args)
#define LOG_GLOB_INFO(args...)                                                         \
  if (auto logger = spdlog::default_logger(); logger->should_log(spdlog::level::info)) \
  logHelper(logger, spdlog::level::info, args)
#define LOG_GLOB_WARN(args...)                                                         \
  if (auto logger = spdlog::default_logger(); logger->should_log(spdlog::level::warn)) \
  logHelper(logger, spdlog::level::warn, args)
#define LOG_GLOB_ERROR(args...)                                                       \
  if (auto logger = spdlog::default_logger(); logger->should_log(spdlog::level::err)) \
  logHelper(logger, spdlog::level::err, args)
#define LOG_GLOB_CRITICAL(args...)                                                         \
  if (auto logger = spdlog::default_logger(); logger->should_log(spdlog::level::critical)) \
  logHelper(logger, spdlog::level::critical, args)
