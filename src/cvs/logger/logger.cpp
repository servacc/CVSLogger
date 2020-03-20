#include "logger.h"

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
  static auto file_dir = fs::current_path() / "log.txt";
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
void Logger::setImageLogEnable(bool enable) { image_log = enable; }

bool Logger::isImageLogEnable() const { return image_log; }

void Logger::log(Level level, const char* name, const cv::Mat& img) {
  log(level, std::string(name), img);
}

void Logger::log(Level level, const std::string& name, const cv::Mat& img) {
  if (isImageLogEnable()) {
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

/*
#include <spdlog/spdlog.h>

#if !defined(__ANDROID__)

#include <experimental/filesystem>
#include <string>

namespace fs = std::experimental::filesystem;

#endif

namespace logging {

//-------------------------------------------------------------------------------------------------

spdlog::level::level_enum convertLogLevel(LogLevel level) {
  switch (level) {
    case LogLevel::trace: return spdlog::level::trace;
    case LogLevel::debug: return spdlog::level::debug;
    case LogLevel::info: return spdlog::level::info;
    case LogLevel::warn: return spdlog::level::warn;
    case LogLevel::err: return spdlog::level::err;
    case LogLevel::critical: return spdlog::level::critical;
    case LogLevel::off: return spdlog::level::off;
  }

  return spdlog::level::off;
}

//-------------------------------------------------------------------------------------------------

LogChannel::LogChannel(std::unique_ptr<spdlog::logger> logger)
    : m_logger(std::move(logger))
    , m_imageLog(ENABLE_LOG_IMAGES_BY_DEFAULT)
    , m_imageExt("png") {}

//-------------------------------------------------------------------------------------------------

LogChannel::~LogChannel() { m_logger->flush(); }

//-------------------------------------------------------------------------------------------------

void LogChannel::setImageExtension(const std::string& ext) { m_imageExt = ext; }
void LogChannel::setImageLogEnable(bool enable) { m_imageLog = enable; }
void LogChannel::setPattern(const std::string& pattern) { m_logger->set_pattern(pattern); }

bool LogChannel::isImageLogEnable() const { return m_imageLog; }
bool LogChannel::isEnabled(LogLevel level) const {
  return m_logger->should_log(convertLogLevel(level));
}

//-------------------------------------------------------------------------------------------------

void LogChannel::setLevel(LogLevel level) {
  m_logger->set_level(convertLogLevel(level));
  setFileSinkEnable(m_logInFile);
}

//-------------------------------------------------------------------------------------------------

void LogChannel::setFileSinkEnable(bool enable) {
  m_logInFile = enable;
  for (auto& s : m_logger->sinks()) {
    auto fSink = std::dynamic_pointer_cast<spdlog::sinks::simple_file_sink_mt>(s);
    if (fSink)
      fSink->set_level(m_logInFile ? m_logger->level() : convertLogLevel(LogLevel::off));
  }
}

//-------------------------------------------------------------------------------------------------

void LogChannel::log(LogLevel level, const char* name, const cv::Mat& img) {
  log(level, std::string(name), img);
}

//-------------------------------------------------------------------------------------------------

void LogChannel::log(LogLevel level, const std::string& name, const cv::Mat& img) {
  if (isImageLogEnable()) {
#if !defined(__ANDROID__)
    auto savePath = fs::path(LogManager::instance().logImagesDir()) / m_logger->name() /
                    spdlog::level::to_str(convertLogLevel(level));
    if (!fs::exists(savePath))
      fs::create_directories(savePath);

    auto filePath =
        (savePath / (name + "_" + std::to_string(m_counters[name]++) + "." + m_imageExt)).string();
#else
    auto filePath = LogManager::instance().logImagesDir() + "/" + m_logger->name() + "_" +
                    spdlog::level::to_str(convertLogLevel(level)) + "_" + name + "_" +
                    std::to_string(m_counters[name]++) + "." + m_imageExt;
#endif
    //        __android_log_print(ANDROID_LOG_INFO, "sometag", "%s", filePath.c_str());

    cv::imwrite(filePath, img);
    log(level, "Image \"{}\" with num {} is saved as \"{}\"", name, m_counters[name] - 1, filePath);
  }
}

//-------------------------------------------------------------------------------------------------

LogManager::LogManager()
    : m_imgPath("./img_log") {}

//-------------------------------------------------------------------------------------------------

LogManager& LogManager::instance() {
  static LogManager manager;
  return manager;
}

//-------------------------------------------------------------------------------------------------

LogChannelPtr LogManager::getLoggerChannel(const std::string& name) {
  auto iter = instance().m_channals.find(name);
  if (iter != instance().m_channals.end())
    return iter->second;

  return nullptr;
}

//-------------------------------------------------------------------------------------------------

LogChannelPtr LogManager::createLoggerChannel(const std::string& name, std::string extraSink) {
  auto iter = instance().m_channals.find(name);
  if (iter != instance().m_channals.end())
    return iter->second;

  std::vector<spdlog::sink_ptr> sinks;

#if !defined(__ANDROID__)
  sinks.push_back(std::make_shared<spdlog::sinks::stdout_sink_mt>());
#else
  sinks.push_back(std::make_shared<spdlog::sinks::android_sink>());
#endif
  if (!extraSink.empty()) {
    auto fi = instance().m_files.find(extraSink);
    if (fi != instance().m_files.end())
      sinks.push_back(fi->second);
    else {
      auto s = std::make_shared<spdlog::sinks::simple_file_sink_mt>(extraSink);
      s->set_force_flush(true);
      sinks.push_back(s);
      instance().m_files.insert({extraSink, s});
    }
  }

  auto logger = std::make_unique<spdlog::logger>(name, std::begin(sinks), std::end(sinks));

  auto channal = std::shared_ptr<LogChannel>(new LogChannel(std::move(logger)));
  instance().m_channals.insert({name, channal});

  return std::move(channal);
}

//-------------------------------------------------------------------------------------------------

LogChannelPtr LogManager::createLoggerChannel(const std::string& name,
                                              LogLevel           level,
                                              std::string        extraSink) {
  auto iter = instance().m_channals.find(name);
  if (iter != instance().m_channals.end())
    return iter->second;

  std::vector<spdlog::sink_ptr> sinks;

#if !defined(__ANDROID__)
  sinks.push_back(std::make_shared<spdlog::sinks::stdout_sink_mt>());
#else
  sinks.push_back(std::make_shared<spdlog::sinks::android_sink>());
#endif
  if (!extraSink.empty()) {
    auto s = std::make_shared<spdlog::sinks::simple_file_sink_mt>(extraSink);
    s->set_force_flush(true);
    sinks.push_back(std::move(s));
  }

  auto logger = std::make_unique<spdlog::logger>(name, std::begin(sinks), std::end(sinks));
  logger->set_pattern("[%H:%M:%S][%n][%L] %v");

  auto channal = std::shared_ptr<LogChannel>(new LogChannel(std::move(logger)));
  instance().m_channals.insert({name, channal});

  channal->setLevel(level);

  return std::move(channal);
}

//-------------------------------------------------------------------------------------------------

void LogManager::setLogImagesDir(const std::string& path) { instance().m_imgPath = path; }

//-------------------------------------------------------------------------------------------------

std::string LogManager::logImagesDir() { return instance().m_imgPath; }

//-------------------------------------------------------------------------------------------------

void LogManager::setLevelAll(LogLevel level) {
  for (auto& c : instance().m_channals) {
    c.second->setLevel(level);
  }
}

//-------------------------------------------------------------------------------------------------

}  // namespace logging

std::istream& operator>>(std::istream& input, logging::LogLevel& x) {
  std::string str;
  input >> str;

  if (str == "trace") {
    x = logging::LogLevel::trace;
  } else if (str == "debug") {
    x = logging::LogLevel::trace;
  } else if (str == "info") {
    x = logging::LogLevel::trace;
  } else if (str == "warn") {
    x = logging::LogLevel::trace;
  } else if (str == "err") {
    x = logging::LogLevel::trace;
  } else if (str == "critical") {
    x = logging::LogLevel::trace;
  } else if (str == "off") {
    x = logging::LogLevel::trace;
  }

  return input;
}
//*/
