#include "../include/cvs/logger/opencvhelper.hpp"

#include <cvs/common/configbase.hpp>
#include <cvs/logger/logging.hpp>

#include <opencv2/imgcodecs.hpp>

namespace {

DECLARE_CONFIG(OpenCVLoggerConfig,
               VALUE(name, std::string),
               VALUE_OPTIONAL(log_img, int),
               VALUE_OPTIONAL(img_path, std::string))

std::string time_point_to_string(const std::chrono::system_clock::time_point& tp) {
  using namespace std;
  using namespace std::chrono;

  auto         ttime_t = system_clock::to_time_t(tp);
  auto         tp_sec  = system_clock::from_time_t(ttime_t);
  milliseconds ms      = duration_cast<milliseconds>(tp - tp_sec);

  std::tm* ttm = localtime(&ttime_t);

  char date_time_format[] = "%Y.%m.%d-%H.%M.%S";

  char time_str[] = "yyyy.mm.dd.HH-MM.SS.fff";

  strftime(time_str, strlen(time_str), date_time_format, ttm);

  string result(time_str);
  result.append(".");
  result.append(to_string(ms.count()));

  return result;
}

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
        confugureLoggerAndOpenCVHelper(logger, c);
      }
    }
  }
}

void confugureLoggerAndOpenCVHelper(std::shared_ptr<spdlog::logger>& logger,
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

const std::string ArgumentPreprocessor<cv::Mat>::subfolder =
    time_point_to_string(std::chrono::system_clock::now());
const spdlog::level::level_enum ArgumentPreprocessor<cv::Mat>::default_save = spdlog::level::info;
const std::filesystem::path     ArgumentPreprocessor<cv::Mat>::default_path =
    std::filesystem::temp_directory_path() / "cvslogger";

std::string ArgumentPreprocessor<cv::Mat>::exec(std::shared_ptr<spdlog::logger>& logger,
                                                spdlog::level::level_enum        lvl,
                                                const cv::Mat&                   arg) {
  auto& info = save_info[logger->name()];

  if (lvl < info.lvl.value_or(default_save))
    return "Image{save disabled}";

  auto save_path = std::filesystem::path(info.path.value_or(default_path)) / subfolder /
                   logger->name() / spdlog::level::to_short_c_str(lvl);
  if (!std::filesystem::exists(save_path))
    std::filesystem::create_directories(save_path);

  auto filepath = save_path / (std::to_string(info.counter++) + ".png");

  if (cv::imwrite(filepath.string(), arg))
    return "cv::Mat(" + std::to_string(arg.cols) + "x" + std::to_string(arg.rows) + "){" +
           filepath.string() + "}";
  return "Image{can't save}";
}

}  // namespace cvs::logger
