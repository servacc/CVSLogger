#include "../include/cvs/logger/logger.h"

#include <filesystem>

#include <spdlog/sinks/stdout_color_sinks.h>
#include <spdlog/sinks/systemd_sink.h>

namespace fs = std::filesystem;

namespace cvs::logger {

spdlog::level::level_enum Logger::default_level = spdlog::level::level_enum::info;

std::shared_ptr<spdlog::logger> Logger::createLogger(std::string name) {
  std::vector<spdlog::sink_ptr> sinks;
  sinks.push_back(std::make_shared<spdlog::sinks::stdout_color_sink_mt>());
  sinks.push_back(std::make_shared<spdlog::sinks::systemd_sink_mt>());

  auto logger = std::make_unique<spdlog::logger>(name, std::begin(sinks), std::end(sinks));
  logger->set_level(Logger::default_level);
  return std::move(logger);
}

LoggerPtr Logger::getLogger(std::string_view name) {
  auto logger = spdlog::get(std::string(name));
  if (!logger) {
    logger = createLogger(std::string(name));
    spdlog::register_logger(logger);
  }

  return std::make_shared<Logger>(std::move(logger));
}

void Logger::initDefaultLogger() {
  static bool already_initialized = false;
  if (!already_initialized) {
    already_initialized = true;

    auto def_logger = createLogger();
    spdlog::set_default_logger(def_logger);
  }
}

spdlog::level::level_enum Logger::convertLogLevel(Level l) {
  switch (l) {
    case Level::trace: return spdlog::level::level_enum::trace;
    case Level::debug: return spdlog::level::level_enum::debug;
    case Level::info: return spdlog::level::level_enum::info;
    case Level::warn: return spdlog::level::level_enum::warn;
    case Level::err: return spdlog::level::level_enum::err;
    case Level::critical: return spdlog::level::level_enum::critical;
    case Level::off: return spdlog::level::level_enum::off;
  }

  return spdlog::level::level_enum::off;
}

spdlog::pattern_time_type Logger::convertTimeType(TimeType tt) {
  switch (tt) {
    case TimeType::local: return spdlog::pattern_time_type::local;
    case TimeType::utc: return spdlog::pattern_time_type::utc;
  }
  return spdlog::pattern_time_type::local;
}

void Logger::setLevelGlobal(Level l) {
  default_level = convertLogLevel(l);
  spdlog::set_level(default_level);
}

void Logger::setPatternGlobal(std::string p, TimeType tt) {
  spdlog::set_pattern(std::move(p), convertTimeType(tt));
}

Logger::Logger(std::shared_ptr<spdlog::logger> logger)
    : logger(std::move(logger)) {}

Logger::~Logger() { logger->flush(); }

void Logger::setPattern(std::string pattern, TimeType tt) {
  logger->set_pattern(std::move(pattern), convertTimeType(tt));
}

bool Logger::isEnabled(Level level) const { return logger->should_log(convertLogLevel(level)); }

void Logger::setLevel(Level level) { logger->set_level(convertLogLevel(level)); }

}  // namespace cvs::logger

#ifdef CVS_LOGGER_OPENCV_ENABLED

#include <opencv2/highgui/highgui.hpp>

namespace cvs::logger {

std::filesystem::path Logger::img_dir = std::filesystem::temp_directory_path();

void Logger::setImgDir(std::filesystem::path p) {
  img_dir = p;
  if (!fs::exists(img_dir)) {
    std::error_code ec;
    if (!fs::create_directories(img_dir, ec))
      LOG_GLOB_CRITICAL("Unable to create log directories {}. {} ({}).", img_dir.string(),
                        ec.message(), ec.value());
  }
}

void Logger::setImageExtension(const std::string& ext) { image_ext = ext; }
void Logger::setLogImage(LogImage enable) { image_log = enable; }

LogImage Logger::logImage() const { return image_log; }

void Logger::log(Level level, const char* name, const cv::Mat& img) {
  log(level, std::string(name), img);
}

void Logger::log(Level level, const std::string& name, const cv::Mat& img) {
  if (logImage() == LogImage::enable) {
    fs::path save_path =
        img_dir / logger->name() / spdlog::level::to_short_c_str(convertLogLevel(level));

    if (!fs::exists(save_path))
      fs::create_directories(save_path);

    auto file_path =
        (save_path / (name + "_" + std::to_string(counters[name]++) + "." + image_ext)).string();

    cv::imwrite(file_path, img);
    log(level, "Image \"{}\" with num {} is saved as \"{}\"", name, counters[name] - 1, file_path);
  }
}

}  // namespace cvs::logger

#endif
