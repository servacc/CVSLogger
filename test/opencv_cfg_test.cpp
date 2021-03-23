#include <gtest/gtest.h>

#include <boost/property_tree/json_parser.hpp>
#include <cvs/common/config.hpp>
#include <cvs/logger/logging.hpp>

namespace {

TEST(CVSLoggerTest, opencv_cfg) {
  auto config_json = R"json(
{
  "loggers": [ {
    "name": "test",
    "log_img": 1,
    "img_path": ")json" TEST_DIR R"json(",
    "level": "0",
    "sink": "1"
  } ]
})json";

  std::stringstream ss;
  ss << config_json;
  boost::property_tree::ptree root;

  ASSERT_NO_THROW(boost::property_tree::read_json(ss, root));

  cvs::common::Config root_cfg(root);

  auto loggers = root_cfg.getChildren("loggers");

  ASSERT_EQ(1, loggers.size());

  cvs::logger::initLoggersAndOpenCVHelper(loggers.front());

  auto logger_path = std::filesystem::path(TEST_DIR) /
                     cvs::logger::ArgumentPreprocessor<cv::Mat>::subfolder / "test";
  if (std::filesystem::exists(logger_path))
    EXPECT_TRUE(std::filesystem::remove_all(logger_path));

  auto logger = cvs::logger::createLogger("test");

  cv::Mat mat(300, 300, CV_8UC3);
  LOG_TRACE(logger, "Trace {}", mat);
  LOG_INFO(logger, "Info {}", mat);

  EXPECT_FALSE(std::filesystem::exists(logger_path / "T" / "0.png"));
  EXPECT_TRUE(std::filesystem::exists(logger_path / "I" / "0.png"));
}

}  // namespace
