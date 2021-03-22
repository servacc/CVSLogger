#include "../include/cvs/logger/opencvhelper.hpp"

#include <cvs/common/configbase.hpp>

#include <opencv2/imgcodecs.hpp>

namespace {

DECLARE_CONFIG(OpenCVLoggerConfig,
               VALUE(name, std::string),
               VALUE_OPTIONAL(log_img, int),
               VALUE_OPTIONAL(img_path, std::string))

}

namespace cvs::logger {

void confugureLoggerForOpenCV(std::shared_ptr<spdlog::logger>& logger, cvs::common::Config& cfg) {
  auto cfg_struct_opt = cfg.parse<OpenCVLoggerConfig>();
  if (cfg_struct_opt) {
    if (cfg_struct_opt->log_img)
      ArgumentPreprocessor<cv::Mat>::save_info[cfg_struct_opt->name].lvl =
          spdlog::level::level_enum(cfg_struct_opt->log_img.value());
    if (cfg_struct_opt->img_path)
      ArgumentPreprocessor<cv::Mat>::save_info[cfg_struct_opt->name].path =
          *cfg_struct_opt->img_path;
  }
}

}  // namespace cvs::logger

namespace cvs::logger {

std::map<std::string, ArgumentPreprocessor<cv::Mat>::LoggerInfo>
    ArgumentPreprocessor<cv::Mat>::save_info;

spdlog::level::level_enum ArgumentPreprocessor<cv::Mat>::default_save = spdlog::level::info;
std::filesystem::path     ArgumentPreprocessor<cv::Mat>::default_path =
    std::filesystem::temp_directory_path() / "cvslogger";

std::string ArgumentPreprocessor<cv::Mat>::exec(std::shared_ptr<spdlog::logger>& logger,
                                                spdlog::level::level_enum        lvl,
                                                const cv::Mat&                   arg) {
  auto& info = save_info[logger->name()];

  if (lvl < info.lvl.value_or(default_save))
    return "Image{save disabled}";

  auto save_path = std::filesystem::path(info.path.value_or(default_path)) / logger->name() /
                   spdlog::level::to_short_c_str(lvl);
  if (!std::filesystem::exists(save_path))
    std::filesystem::create_directories(save_path);

  auto filepath = save_path / (std::to_string(info.counter++) + ".png");

  if (cv::imwrite(filepath.string(), arg))
    return "cv::Mat(" + std::to_string(arg.cols) + "x" + std::to_string(arg.rows) + "){" +
           filepath.string() + "}";
  return "Image{can't save}";
}

}  // namespace cvs::logger
