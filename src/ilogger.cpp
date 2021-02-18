#include "../include/cvs/logger/ilogger.hpp"

#ifdef CVS_LOGGER_OPENCV_ENABLED

namespace cvs::logger {

template <>
ILogger::Strategy<cv::Mat>::Type ILogger::processArg<cv::Mat>(Level l, const cv::Mat& arg) {
  static std::atomic_size_t id{0};
  if (logImage() == LogImage::enable) {
    auto save_path = path() / "images" / name() / std::to_string(int(l));
    std::filesystem::create_directories(save_path);
    save_path /= std::to_string(id++) + ".png";
    cv::imwrite(save_path.string(), arg);

    return "Img(" + save_path.string() + ")";
  }

  return std::string("Img(not saved)");
}

}  // namespace cvs::logger

#endif
