#include <gtest/gtest.h>

#include <cvs/logger/logger.h>

using namespace cvs::logger;

namespace {

TEST(CVSLoggerTest, glogal_logger) {
  Logger::configure(Level::trace);

  LOG_GLOB_TRACE("Test message level num {}", Level::trace);
  LOG_GLOB_DEBUG("Test message level num {}", Level::debug);
  LOG_GLOB_INFO("Test message level num {}", Level::info);
  LOG_GLOB_WARN("Test message level num {}", Level::warn);
  LOG_GLOB_ERROR("Test message level num {}", Level::err);
  LOG_GLOB_CRITICAL("Test message level num {}", Level::critical);
}

}  // namespace

#ifdef CVS_LOGGER_OPENCV_ENABLED

#include <opencv2/imgcodecs.hpp>
#include <opencv2/imgproc.hpp>

#include <opencv2/highgui.hpp>

namespace {

void drawRandomLines(cv::Mat& image, cv::RNG rng = {}) {
  constexpr int lineType   = 8;
  constexpr int line_count = 8;
  cv::Point     pt1, pt2;
  for (int i = 0; i < line_count; ++i) {
    pt1.x = rng.uniform(0, image.cols);
    pt1.y = rng.uniform(0, image.rows);
    pt2.x = rng.uniform(0, image.cols);
    pt2.y = rng.uniform(0, image.rows);
    cv::line(image, pt1, pt2,
             cv::Scalar(rng.uniform(0, 255), rng.uniform(0, 255), rng.uniform(0, 255)),
             rng.uniform(1, 10), lineType);
  }
}

TEST(CVSLoggerTest, opencv) {
  cv::Mat mat(300, 300, CV_8UC3, cv::Scalar(0));
  drawRandomLines(mat);

  auto logger = Logger::getLogger("cvs.logger.test");
  logger->setLogImage(LogImage::enable);
  LOG_INFO(logger, "test1", mat);

  ASSERT_TRUE(std::filesystem::exists("/tmp/cvs.logger.test/I/test1_0.png"));
}

}  // namespace

#endif
