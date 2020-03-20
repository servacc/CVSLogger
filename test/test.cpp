#include <gtest/gtest.h>

#include <cvs/logger/logger.h>

using namespace cvs::logger;

namespace {

TEST(CVSLoggerTest, test) {
  Logger::configure(Level::trace);

  LOG_GLOB_TRACE("Test message level num {}", Level::trace);
  LOG_GLOB_DEBUG("Test message level num {}", Level::debug);
  LOG_GLOB_INFO("Test message level num {}", Level::info);
  LOG_GLOB_WARN("Test message level num {}", Level::warn);
  LOG_GLOB_ERROR("Test message level num {}", Level::err);
  LOG_GLOB_CRITICAL("Test message level num {}", Level::critical);
}

}  // namespace
