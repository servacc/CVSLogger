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

}  // namespace
