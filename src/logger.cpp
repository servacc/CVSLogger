#include "../include/cvs/logger/logger.h"

#include <filesystem>

#include <spdlog/sinks/basic_file_sink.h>
#include <spdlog/sinks/stdout_color_sinks.h>

namespace fs = std::filesystem;

namespace cvs::logger {

spdlog::level::level_enum Logger::default_level = spdlog::level::level_enum::info;

std::shared_ptr<spdlog::logger> Logger::createLogger(std::string name) {
  std::vector<spdlog::sink_ptr> sinks;
  sinks.push_back(std::make_shared<spdlog::sinks::stdout_color_sink_mt>());
  sinks.push_back(std::make_shared<spdlog::sinks::basic_file_sink_mt>(Logger::logDir().string()));

  auto logger = std::make_unique<spdlog::logger>(name, std::begin(sinks), std::end(sinks));
  logger->set_level(Logger::default_level);
  return std::move(logger);
}

ChannalPtr Logger::getLogger(const std::string& name) {
  auto logger = spdlog::get(name);
  if (!logger) {
    logger = createLogger(name);
    spdlog::register_logger(logger);
  }

  return std::make_shared<Logger>(std::move(logger));
}

std::filesystem::path Logger::logDir(std::filesystem::path path) {
  static auto file_dir = fs::temp_directory_path() / "log.txt";
  if (!path.empty())
    file_dir = path;

  return file_dir;
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

void Logger::setLevel(Level level) {
  logger->set_level(convertLogLevel(level));
  setFileSinkEnable(log_in_file);
}

void Logger::setFileSinkEnable(bool enable) {
  log_in_file = enable;
  for (auto& s : logger->sinks()) {
    auto f_sink = std::dynamic_pointer_cast<spdlog::sinks::basic_file_sink_mt>(s);
    if (f_sink) {
      f_sink->set_level(log_in_file ? logger->level() : convertLogLevel(Level::off));
    }
  }
}

}  // namespace cvs::logger

#ifdef CVS_LOGGER_OPENCV_ENABLED

#include <opencv2/highgui/highgui.hpp>

namespace cvs::logger {

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
