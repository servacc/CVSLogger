#pragma once

#include <cvs/logger/ilogger.hpp>
#include <cvs/logger/loggerfactory.hpp>

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

#define LOG_GLOB_TRACE(args...)                                  \
  if (auto logger = cvs::logger::LoggerFactory::defaultLogger(); \
      logger->isEnabled(cvs::logger::Level::trace))              \
  logger->log(cvs::logger::Level::trace, args)
#define LOG_GLOB_DEBUG(args...)                                  \
  if (auto logger = cvs::logger::LoggerFactory::defaultLogger(); \
      logger->isEnabled(cvs::logger::Level::debug))              \
  logger->log(cvs::logger::Level::debug, args)
#define LOG_GLOB_INFO(args...)                                   \
  if (auto logger = cvs::logger::LoggerFactory::defaultLogger(); \
      logger->isEnabled(cvs::logger::Level::info))               \
  logger->log(cvs::logger::Level::info, args)
#define LOG_GLOB_WARN(args...)                                   \
  if (auto logger = cvs::logger::LoggerFactory::defaultLogger(); \
      logger->isEnabled(cvs::logger::Level::warn))               \
  logger->log(cvs::logger::Level::warn, args)
#define LOG_GLOB_ERROR(args...)                                  \
  if (auto logger = cvs::logger::LoggerFactory::defaultLogger(); \
      logger->isEnabled(cvs::logger::Level::err))                \
  logger->log(cvs::logger::Level::err, args)
#define LOG_GLOB_CRITICAL(args...)                               \
  if (auto logger = cvs::logger::LoggerFactory::defaultLogger(); \
      logger->isEnabled(cvs::logger::Level::critical))           \
  logger->log(cvs::logger::Level::critical, args)
