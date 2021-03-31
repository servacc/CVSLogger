#pragma once

#include <spdlog/spdlog.h>
#include <cvs/common/config.hpp>
#include <cvs/logger/argumentpreprocessor.hpp>

#ifdef CVS_LOGGER_OPENCV_ENABLED
#include <cvs/logger/opencvhelper.hpp>
#endif

namespace cvs::logger {

using LoggerPtr = std::shared_ptr<spdlog::logger>;

bool initLoggers(std::optional<cvs::common::Config> = std::nullopt);

void registerLoggersInFactory();
void createDefaultLogger(std::optional<cvs::common::Config>);

LoggerPtr createLogger(const std::string&, std::optional<cvs::common::Config> = std::nullopt);
bool      configureLogger(LoggerPtr, cvs::common::Config&);

namespace detail {

template <typename... Args>
void logHelper(LoggerPtr&                logger,
               spdlog::level::level_enum lvl,
               spdlog::string_view_t     fmt,
               Args&&... args) {
  if (logger && logger->should_log(lvl))
    logger->log(lvl, fmt,
                (ArgumentPreprocessor<std::remove_cvref_t<Args>>::exec(
                    logger, lvl, std::forward<Args>(args)))...);
}

template <typename... Args>
void logHelper(spdlog::level::level_enum lvl, Args&&... args) {
  auto logger = spdlog::default_logger();
  logHelper(logger, lvl, std::forward<Args>(args)...);
}

}  // namespace detail

}  // namespace cvs::logger

#define LOG_TRACE(CH, ...) cvs::logger::detail::logHelper(CH, spdlog::level::trace, __VA_ARGS__)
#define LOG_DEBUG(CH, ...) cvs::logger::detail::logHelper(CH, spdlog::level::debug, __VA_ARGS__)
#define LOG_INFO(CH, ...) cvs::logger::detail::logHelper(CH, spdlog::level::info, __VA_ARGS__)
#define LOG_WARN(CH, ...) cvs::logger::detail::logHelper(CH, spdlog::level::warn, __VA_ARGS__)
#define LOG_ERROR(CH, ...) cvs::logger::detail::logHelper(CH, spdlog::level::err, __VA_ARGS__)
#define LOG_CRITICAL(CH, ...) \
  cvs::logger::detail::logHelper(CH, spdlog::level::critical, __VA_ARGS__)

#define LOG_GLOB_TRACE(...) cvs::logger::detail::logHelper(spdlog::level::trace, __VA_ARGS__)
#define LOG_GLOB_DEBUG(...) cvs::logger::detail::logHelper(spdlog::level::debug, __VA_ARGS__)
#define LOG_GLOB_INFO(...) cvs::logger::detail::logHelper(spdlog::level::info, __VA_ARGS__)
#define LOG_GLOB_WARN(...) cvs::logger::detail::logHelper(spdlog::level::warn, __VA_ARGS__)
#define LOG_GLOB_ERROR(...) cvs::logger::detail::logHelper(spdlog::level::err, __VA_ARGS__)
#define LOG_GLOB_CRITICAL(...) cvs::logger::detail::logHelper(spdlog::level::critical, __VA_ARGS__)
