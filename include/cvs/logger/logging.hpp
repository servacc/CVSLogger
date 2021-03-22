#pragma once

#include <spdlog/spdlog.h>
#include <cvs/common/config.hpp>
#include <cvs/logger/argumentpreprocessor.hpp>

#ifdef CVS_LOGGER_OPENCV_ENABLED
#include <filesystem>
#include <map>
#include <opencv2/core/mat.hpp>
#endif

namespace cvs::logger {

using LoggerPtr = std::shared_ptr<spdlog::logger>;

void initLoggers(std::optional<cvs::common::Config> = std::nullopt);

void registerLoggersInFactory();
void createDefaultLogger(std::optional<cvs::common::Config>);

LoggerPtr createLogger(std::string, std::optional<cvs::common::Config> = std::nullopt);
void      configureLogger(LoggerPtr, cvs::common::Config&);

#ifdef CVS_LOGGER_OPENCV_ENABLED

template <>
struct ArgumentPreprocessor<cv::Mat> {
  static std::string exec(LoggerPtr&, spdlog::level::level_enum, const cv::Mat& arg);

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
void logHelper(LoggerPtr&                logger,
               spdlog::level::level_enum lvl,
               spdlog::string_view_t     fmt,
               Args&&... args) {
  logger->log(lvl, fmt,
              (ArgumentPreprocessor<std::remove_cvref_t<Args>>::exec(logger, lvl,
                                                                     std::forward<Args>(args)))...);
}

}  // namespace cvs::logger

#define LOG_TRACE(CH, ...)                        \
  if (CH && CH->should_log(spdlog::level::trace)) \
  cvs::logger::logHelper(CH, spdlog::level::trace, __VA_ARGS__)
#define LOG_DEBUG(CH, ...)                        \
  if (CH && CH->should_log(spdlog::level::debug)) \
  cvs::logger::logHelper(CH, spdlog::level::debug, __VA_ARGS__)
#define LOG_INFO(CH, ...)                        \
  if (CH && CH->should_log(spdlog::level::info)) \
  cvs::logger::logHelper(CH, spdlog::level::info, __VA_ARGS__)
#define LOG_WARN(CH, ...)                        \
  if (CH && CH->should_log(spdlog::level::warn)) \
  cvs::logger::logHelper(CH, spdlog::level::warn, __VA_ARGS__)
#define LOG_ERROR(CH, ...)                      \
  if (CH && CH->should_log(spdlog::level::err)) \
  cvs::logger::logHelper(CH, spdlog::level::err, __VA_ARGS__)
#define LOG_CRITICAL(CH, ...)                        \
  if (CH && CH->should_log(spdlog::level::critical)) \
  cvs::logger::logHelper(CH, spdlog::level::critical, __VA_ARGS__)

#define LOG_GLOB_TRACE(args...)                                                         \
  if (auto logger = spdlog::default_logger(); logger->should_log(spdlog::level::trace)) \
  cvs::logger::logHelper(logger, spdlog::level::trace, args)
#define LOG_GLOB_DEBUG(args...)                                                         \
  if (auto logger = spdlog::default_logger(); logger->should_log(spdlog::level::debug)) \
  cvs::logger::logHelper(logger, spdlog::level::debug, args)
#define LOG_GLOB_INFO(args...)                                                         \
  if (auto logger = spdlog::default_logger(); logger->should_log(spdlog::level::info)) \
  cvs::logger::logHelper(logger, spdlog::level::info, args)
#define LOG_GLOB_WARN(args...)                                                         \
  if (auto logger = spdlog::default_logger(); logger->should_log(spdlog::level::warn)) \
  cvs::logger::logHelper(logger, spdlog::level::warn, args)
#define LOG_GLOB_ERROR(args...)                                                       \
  if (auto logger = spdlog::default_logger(); logger->should_log(spdlog::level::err)) \
  cvs::logger::logHelper(logger, spdlog::level::err, args)
#define LOG_GLOB_CRITICAL(args...)                                                         \
  if (auto logger = spdlog::default_logger(); logger->should_log(spdlog::level::critical)) \
  cvs::logger::logHelper(logger, spdlog::level::critical, args)
