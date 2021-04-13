#include "../include/cvs/logger/opencvhelper.hpp"

#include <cvs/common/configbase.hpp>
#include <cvs/logger/logging.hpp>

#include <fmt/chrono.h>

#include <opencv2/imgcodecs.hpp>

namespace {

CVSCFG_DECLARE_CONFIG(OpenCVLoggerConfig,
                      CVSCFG_VALUE(name, std::string),
                      CVSCFG_VALUE_OPTIONAL(log_img, int),
                      CVSCFG_VALUE_OPTIONAL(img_path, std::string))

}  // namespace

namespace cvs::logger {

void initLoggersAndOpenCVHelper(std::optional<cvs::common::Config> config) {
  initLoggers(config);

  if (config) {
    auto loggers = config->getChildren();
    for (auto c : loggers) {
      auto logger_conf = c.parse<OpenCVLoggerConfig>();
      if (logger_conf) {
        auto logger = createLogger(logger_conf->name);
        configureLoggerAndOpenCVHelper(logger, c);
      }
    }
  }
}

void configureLoggerAndOpenCVHelper(std::shared_ptr<spdlog::logger>& logger,
                                    cvs::common::Config&             cfg) {
  auto cfg_struct_opt = cfg.parse<OpenCVLoggerConfig>();
  if (cfg_struct_opt) {
    if (cfg_struct_opt->log_img)
      ArgumentPreprocessor<cv::Mat>::save_info[cfg_struct_opt->name].lvl =
          spdlog::level::level_enum(cfg_struct_opt->log_img.value());
    if (cfg_struct_opt->img_path)
      ArgumentPreprocessor<cv::Mat>::save_info[cfg_struct_opt->name].path =
          *cfg_struct_opt->img_path;
  }

  configureLogger(logger, cfg);
}

}  // namespace cvs::logger

namespace cvs::logger {

std::map<std::string, ArgumentPreprocessor<cv::Mat>::LoggerInfo>
    ArgumentPreprocessor<cv::Mat>::save_info;

std::string ArgumentPreprocessor<cv::Mat>::exec(std::shared_ptr<spdlog::logger>& logger,
                                                spdlog::level::level_enum        lvl,
                                                const cv::Mat&                   arg) {
  auto& info = save_info[logger->name()];

  if (lvl < info.lvl.value_or(defaultSave()))
    return "Image{save disabled}";

  auto save_path = std::filesystem::path(info.path.value_or(defaultPath())) / subfolder() /
                   logger->name() / spdlog::level::to_short_c_str(lvl);
  if (!std::filesystem::exists(save_path))
    std::filesystem::create_directories(save_path);

  auto filepath = save_path / (std::to_string(info.counter++) + ".png");

  if (cv::imwrite(filepath.string(), arg))
    return "cv::Mat(" + std::to_string(arg.cols) + "x" + std::to_string(arg.rows) + "){" +
           filepath.string() + "}";
  return "Image{can't save}";
}

const std::string& ArgumentPreprocessor<cv::Mat>::subfolder() {
  static const std::string name = fmt::format(
      "{:%Y.%m.%d-%H.%M.%S}",
      fmt::localtime(std::chrono::system_clock::to_time_t(std::chrono::system_clock::now())));

  return name;
}

spdlog::level::level_enum ArgumentPreprocessor<cv::Mat>::defaultSave() {
  static const spdlog::level::level_enum default_save = spdlog::level::info;
  return default_save;
}

const std::filesystem::path& ArgumentPreprocessor<cv::Mat>::defaultPath() {
  static const std::filesystem::path default_path =
      std::filesystem::temp_directory_path() / "cvslogger";
  return default_path;
}

}  // namespace cvs::logger
