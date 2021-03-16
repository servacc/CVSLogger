#include <gtest/gtest.h>

#include <cvs/logger/logging.hpp>

#include <opencv2/highgui.hpp>
#include <opencv2/imgcodecs.hpp>
#include <opencv2/imgproc.hpp>

using namespace cvs::logger;

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

  LoggerFactory::configure("test.logger", std::tuple{LogImage::enable, Sinks::STDOUT});

  auto logger = LoggerFactory::getLogger("test.logger");
  //  LOG_INFO(logger, "Save to {}", mat);

  ASSERT_TRUE(std::filesystem::exists("/tmp/images/test.logger/2/0.png"));
}

}  // namespace
