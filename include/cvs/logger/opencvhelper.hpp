#pragma once

#include <filesystem>
#include <map>
#include <opencv2/core/mat.hpp>

#include <cvs/common/config.hpp>
#include <cvs/logger/argumentpreprocessor.hpp>

namespace cvs::logger {

void initLoggersAndOpenCVHelper(std::optional<cvs::common::Config> = std::nullopt);
void configureLoggerAndOpenCVHelper(std::shared_ptr<spdlog::logger>& logger, cvs::common::Config&);

template <>
struct ArgumentPreprocessor<cv::Mat> {
  static std::string exec(std::shared_ptr<spdlog::logger>&,
                          spdlog::level::level_enum,
                          const cv::Mat& arg);

  static const std::string&           subfolder();
  static spdlog::level::level_enum    defaultSave();
  static const std::filesystem::path& defaultPath();

  struct LoggerInfo {
    std::size_t                              counter = 0;
    std::optional<spdlog::level::level_enum> lvl;
    std::optional<std::filesystem::path>     path;
  };

  static std::map<std::string, LoggerInfo> save_info;
};

}  // namespace cvs::logger
