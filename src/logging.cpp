#include "../include/cvs/logger/logging.hpp"

#include <spdlog/sinks/stdout_color_sinks.h>
#include <spdlog/sinks/systemd_sink.h>
#include <spdlog/spdlog.h>

#include <cvs/common/configbase.hpp>
#include <cvs/common/factory.hpp>

#ifdef CVS_LOGGER_OPENCV_ENABLED
#include <opencv2/imgcodecs.hpp>
#endif

using namespace cvs::logger;
using namespace std::string_literals;

namespace {

DECLARE_CONFIG(LoggerConfig,
               VALUE(name, std::string),
               VALUE_OPTIONAL(log_img, int),
               VALUE_OPTIONAL(img_path, std::string),
               VALUE_OPTIONAL(level, int),
               VALUE_OPTIONAL(pattern, std::string),
               VALUE_OPTIONAL(time_type, int),
               VALUE_OPTIONAL(sink, int))

using StdoutSink  = spdlog::sinks::stdout_color_sink_mt;
using SystemdSink = spdlog::sinks::systemd_sink_st;

static Sinks default_sinks =
#ifdef CVSLOGGER_STD_ENABLED
#ifdef CVSLOGGER_SYSD_ENABLED
    Sinks::SYSTEMD | Sinks::STDOUT;
#else
    Sinks::STDOUT;
#endif
#else
#ifdef CVSLOGGER_SYSD_ENABLED
    Sinks::SYSTEMD;
#else
    Sinks::NOSINK;
#endif
#endif

template <typename SinkType>
auto createSink(bool enable) {
  auto sink = std::make_shared<SinkType>();
  if (!enable)
    sink->set_level(spdlog::level::off);
  return sink;
}

cvs::logger::Logger1Ptr createLogger(std::string name) {
  std::vector<spdlog::sink_ptr> sinks;
  sinks.push_back(createSink<StdoutSink>(default_sinks & cvs::logger::Sinks::STDOUT));
  sinks.push_back(createSink<SystemdSink>(default_sinks & cvs::logger::Sinks::SYSTEMD));

  return std::make_shared<spdlog::logger>(name, std::begin(sinks), std::end(sinks));
}

cvs::logger::Logger1Ptr getOrCreateLogger(std::string name) {
  auto logger = spdlog::get(name);
  if (!logger) {
    logger = createLogger(name);
    spdlog::register_logger(logger);
  }
  return logger;
}

void configureLogger(cvs::logger::Logger1Ptr& logger, LoggerConfig& config) {
#ifdef CVS_LOGGER_OPENCV_ENABLED
  if (config.log_img)
    ProcessArg<cv::Mat>::save_info[config.name].lvl =
        spdlog::level::level_enum(config.log_img.value());
  if (config.img_path)
    ProcessArg<cv::Mat>::save_info[config.name].path = *config.img_path;
#endif

  if (config.pattern) {
    auto tt = config.time_type.value_or((int)spdlog::pattern_time_type::local);
    logger->set_pattern(*config.pattern, spdlog::pattern_time_type(tt));
  }

  auto level = spdlog::level::level_enum(config.level.value_or(spdlog::level::info));
  logger->set_level(level);

  if (config.sink) {
    auto sinks_flags = Sinks(*config.sink);
    auto sinks       = logger->sinks();
    for (auto& s : sinks) {
      auto std_sink = std::dynamic_pointer_cast<StdoutSink>(s);
      if (std_sink) {
        std_sink->set_level(sinks_flags & Sinks::STDOUT ? level : spdlog::level::off);
        continue;
      }
      auto sys_sink = std::dynamic_pointer_cast<SystemdSink>(s);
      if (sys_sink) {
        sys_sink->set_level(sinks_flags & Sinks::SYSTEMD ? level : spdlog::level::off);
        continue;
      }
    }
  }
}

}  // namespace

namespace cvs::logger {

void initDefaultLogger(std::optional<cvs::common::Config> config) {
  auto default_logger = createLogger("");
  spdlog::set_default_logger(default_logger);

  if (config) {
    if (auto c = config->parse<LoggerConfig>(); c)
      configureLogger(default_logger, *c);
  }
}

void initLoggers(std::optional<common::Config> config) {
  registerLogger();
  initDefaultLogger(config);
}

void registerLogger() {
  common::Factory::registrate<Logger1Ptr(std::string)>("cvslogger"s, getOrCreateLogger);

  common::Factory::registrate<Logger1Ptr(cvs::common::Config)>(
      "cvslogger"s, [](cvs::common::Config cfg) -> Logger1Ptr {
        auto cfg_struct_opt = cfg.parse<LoggerConfig>();
        if (cfg_struct_opt) {
          auto logger_opt = common::Factory::create<Logger1Ptr>("cvslogger"s, cfg_struct_opt->name);
          if (!logger_opt)
            return {};
          configureLogger(*logger_opt, *cfg_struct_opt);
          return *logger_opt;
        }

        return {};
      });
}

}  // namespace cvs::logger

#ifdef CVS_LOGGER_OPENCV_ENABLED

namespace cvs::logger {

std::map<std::string, ProcessArg<cv::Mat>::LoggerInfo> ProcessArg<cv::Mat>::save_info;

spdlog::level::level_enum ProcessArg<cv::Mat>::default_save = spdlog::level::info;
std::filesystem::path ProcessArg<cv::Mat>::default_path = std::filesystem::temp_directory_path();

std::string ProcessArg<cv::Mat>::exec(Logger1Ptr&               logger,
                                      spdlog::level::level_enum lvl,
                                      const cv::Mat&            arg) {
  auto info = save_info[logger->name()];

  if (lvl < info.lvl.value_or(default_save))
    return "Image{save disabled}";

  auto filepath =
      info.path.value_or(default_path) / (logger->name() + std::to_string(info.counter++) + ".png");

  if (cv::imwrite(filepath.string(), arg))
    return "Image{" + filepath.string() + "}";
  return "Image{can't save}";
}

}  // namespace cvs::logger

#endif
