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

class CreateLoggersTest : public ::testing::WithParamInterface<std::string> {
 public:
  static void SetUpTestSuite() { cvs::logger::initLoggers(); }
};

TEST_P(CreateLoggersTest, configure) {
  // clang-format off
  std::string config_json =
    R"json(
    {
      "logger": {
        "name": "",
        "level": "0",
        "sink": "1"
      }
    })json";
  // clang-format on

  std::stringstream ss;
  ss << config_json;
  boost::property_tree::ptree root;

  ASSERT_NO_THROW(boost::property_tree::read_json(ss, root));

  std::optional<boost::property_tree::ptree> logger_json =
      cvs::common::utils::boostOptionalToStd(root.get_child_optional("logger"));
  ASSERT_TRUE(logger_json);

  cvs::common::Config cfg(*logger_json);

  auto logger_opt = cvs::common::Factory::create<Logger1Ptr>("cvslogger"s, cfg);
  ASSERT_TRUE(logger_opt);
  ASSERT_NE(nullptr, *logger_opt);

  LOG_GLOB_TRACE("Test 0");
  LOG_GLOB_DEBUG("Test 0");
  LOG_GLOB_INFO("Test 0");
  LOG_GLOB_WARN("Test 0");
  LOG_GLOB_ERROR("Test 0");

  //  LoggerFactory::configure(LoggerFactory::default_logger_name, std::tuple{Level::info});

  //  LOG_GLOB_TRACE("Test 1");
  //  LOG_GLOB_DEBUG("Test 1");
  //  LOG_GLOB_INFO("Test 1");
  //  LOG_GLOB_WARN("Test 1");
  //  LOG_GLOB_ERROR("Test 1");

  //  LoggerFactory::configure(LoggerFactory::default_logger_name,
  //                           std::tuple{Level::err, Sinks::STDOUT});

  //  LOG_GLOB_TRACE("Test 2");
  //  LOG_GLOB_DEBUG("Test 2");
  //  LOG_GLOB_INFO("Test 2");
  //  LOG_GLOB_WARN("Test 2");
  //  LOG_GLOB_ERROR("Test 2");
}

INSTANTIATE_TEST_SUITE_P(LoggerTest, CreateLoggersTest, ::testing::Values(R"json(
    {
      "logger": {
        "name": "",
        "level": "0",
        "sink": "1"
      }
    })json"));

/*
TEST(DefraultFactoryTest, logger) {
  LoggerFactory::configure("test-logger", std::tuple{Level::trace, Sinks::STDOUT |
Sinks::SYSTEMD});

  auto logger = LoggerFactory::getLogger("test-logger");

  LOG_TRACE(logger, "Test 0");
  LOG_DEBUG(logger, "Test 0");
  LOG_INFO(logger, "Test 0");
  LOG_WARN(logger, "Test 0");
  LOG_ERROR(logger, "Test 0");
}

TEST(DefraultFactoryTest, regex) {
  LoggerFactory::configure(Regex{"test.*"},
                           std::tuple{Pattern{"[%=10n] [%=7l] %v", TimeType::utc},
Sinks::STDOUT}); LoggerFactory::configure("test.trace", std::tuple{Level::trace}, "test.info",
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
//*/
}  // namespace
