#include <gtest/gtest.h>

#include <cvs/logger/logging.hpp>

#include <list>
#include <regex>
#include <tuple>

using namespace cvs::logger;

TEST(DefraultFactoryTest, defalt_logger) {
  LoggerFactory::configure(LoggerFactory::default_logger_name,
                           std::tuple{Level::trace, Sinks::STDOUT | Sinks::SYSTEMD});

  LOG_GLOB_TRACE("Test 0");
  LOG_GLOB_DEBUG("Test 0");
  LOG_GLOB_INFO("Test 0");
  LOG_GLOB_WARN("Test 0");
  LOG_GLOB_ERROR("Test 0");

  LoggerFactory::configure(LoggerFactory::default_logger_name, std::tuple{Level::info});

  LOG_GLOB_TRACE("Test 1");
  LOG_GLOB_DEBUG("Test 1");
  LOG_GLOB_INFO("Test 1");
  LOG_GLOB_WARN("Test 1");
  LOG_GLOB_ERROR("Test 1");

  LoggerFactory::configure(LoggerFactory::default_logger_name,
                           std::tuple{Level::err, Sinks::STDOUT});

  LOG_GLOB_TRACE("Test 2");
  LOG_GLOB_DEBUG("Test 2");
  LOG_GLOB_INFO("Test 2");
  LOG_GLOB_WARN("Test 2");
  LOG_GLOB_ERROR("Test 2");
}

TEST(DefraultFactoryTest, logger) {
  LoggerFactory::configure("test-logger", std::tuple{Level::trace, Sinks::STDOUT | Sinks::SYSTEMD});

  auto logger = LoggerFactory::getLogger("test-logger");

  LOG_TRACE(logger, "Test 0");
  LOG_DEBUG(logger, "Test 0");
  LOG_INFO(logger, "Test 0");
  LOG_WARN(logger, "Test 0");
  LOG_ERROR(logger, "Test 0");
}

TEST(DefraultFactoryTest, regex) {
  LoggerFactory::configure(Regex{"test.*"},
                           std::tuple{Pattern{"[%=10n] [%=7l] %v", TimeType::utc}, Sinks::STDOUT});
  LoggerFactory::configure("test.trace", std::tuple{Level::trace}, "test.info",
                           std::tuple{Level::info});

  auto logger0 = LoggerFactory::getLogger("test.trace");
  auto logger1 = LoggerFactory::getLogger("test.info");

  LOG_TRACE(logger0, "test");
  LOG_DEBUG(logger0, "test");
  LOG_INFO(logger0, "test");
  LOG_WARN(logger0, "test");
  LOG_ERROR(logger0, "test");

  LOG_TRACE(logger1, "test");
  LOG_DEBUG(logger1, "test");
  LOG_INFO(logger1, "test");
  LOG_WARN(logger1, "test");
  LOG_ERROR(logger1, "test");
}
