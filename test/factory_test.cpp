#include <gtest/gtest.h>

#include <cvs/common/configbase.hpp>
#include <cvs/common/factory.hpp>
#include <cvs/logger/logging.hpp>

#include <list>
#include <regex>
#include <tuple>

using namespace cvs::logger;
using namespace std::string_literals;

namespace {

class DefaultLoggerTest : public ::testing::TestWithParam<std::string> {
 public:
  static void SetUpTestSuite() { cvs::logger::initLoggers(); }
};

TEST_P(DefaultLoggerTest, configure) {
  std::string config_json = GetParam();

  std::stringstream ss;
  ss << config_json;
  boost::property_tree::ptree root;

  ASSERT_NO_THROW(boost::property_tree::read_json(ss, root));

  std::optional<boost::property_tree::ptree> logger_json =
      cvs::common::utils::boostOptionalToStd(root.get_child_optional("logger"));
  ASSERT_TRUE(logger_json);

  cvs::common::Config cfg(*logger_json);

  cvs::logger::configureLogger(spdlog::default_logger(), cfg);

  LOG_GLOB_TRACE("Test {} {}", 0, 1);
  LOG_GLOB_DEBUG("Test {} {}", 0, 1);
  LOG_GLOB_INFO("Test {} {}", 0, 1);
  LOG_GLOB_WARN("Test {} {}", 0, 1);
  LOG_GLOB_ERROR("Test {} {}", 0, 1);
}

INSTANTIATE_TEST_SUITE_P(LoggerTest,
                         DefaultLoggerTest,
                         ::testing::Values(
                             R"json(
{
  "logger": {
    "name": "",
    "level": "0",
    "sink": "1"
  }
})json",
                             R"json(
{
  "logger": {
    "name": "",
    "level": "2",
    "sink": "1"
  }
})json"));

TEST(LoggerTest, config_array) {
  std::string config_json = R"json(
{
  "item1" : { "name": "1" },
  "loggers": [
    { "name": "", "level": "0", "sink": "1" },
    { "name": "test0", "level": "0", "sink": "1" },
    { "name": "test1", "level": "1", "sink": "1" },
    { "name": "test2", "level": "2", "sink": "1" }
  ],
  "item1" : { "name": "2" }
})json";

  std::stringstream ss;
  ss << config_json;
  boost::property_tree::ptree root;

  ASSERT_NO_THROW(boost::property_tree::read_json(ss, root));

  cvs::common::Config root_cfg(root);

  auto loggers = root_cfg.getChildren("loggers");

  ASSERT_EQ(1, loggers.size());

  cvs::logger::initLoggers(loggers.front());

  LOG_GLOB_TRACE("DEF {} {}", 0, 1);
  LOG_GLOB_DEBUG("DEF {} {}", 0, 1);
  LOG_GLOB_INFO("DEF {} {}", 0, 1);
  LOG_GLOB_WARN("DEF {} {}", 0, 1);
  LOG_GLOB_ERROR("DEF {} {}", 0, 1);

  auto logger0 = createLogger("test0");

  LOG_TRACE(logger0, "test0 {} {}", 0, 1);
  LOG_DEBUG(logger0, "test0 {} {}", 0, 1);
  LOG_INFO(logger0, "test0 {} {}", 0, 1);
  LOG_WARN(logger0, "test0 {} {}", 0, 1);
  LOG_ERROR(logger0, "test0 {} {}", 0, 1);

  auto logger1 = createLogger("test1");

  LOG_TRACE(logger1, "test1 {} {}", 0, 1);
  LOG_DEBUG(logger1, "test1 {} {}", 0, 1);
  LOG_INFO(logger1, "test1 {} {}", 0, 1);
  LOG_WARN(logger1, "test1 {} {}", 0, 1);
  LOG_ERROR(logger1, "test1 {} {}", 0, 1);

  auto logger2 = createLogger("test2");

  LOG_TRACE(logger2, "test2 {} {}", 0, 1);
  LOG_DEBUG(logger2, "test2 {} {}", 0, 1);
  LOG_INFO(logger2, "test2 {} {}", 0, 1);
  LOG_WARN(logger2, "test2 {} {}", 0, 1);
  LOG_ERROR(logger2, "test2 {} {}", 0, 1);
}

}  // namespace
