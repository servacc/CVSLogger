#include "defaultfactory.hpp"
#include "../include/cvs/logger/ilogger.hpp"

#include <spdlog/sinks/stdout_color_sinks.h>
#include <spdlog/sinks/systemd_sink.h>
#include <spdlog/spdlog.h>

#include <regex>

using namespace cvs::logger;

namespace {

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

spdlog::pattern_time_type convertTimeType(TimeType tt) {
  switch (tt) {
    case TimeType::local: return spdlog::pattern_time_type::local;
    case TimeType::utc: return spdlog::pattern_time_type::utc;
  }
  return spdlog::pattern_time_type::local;
}

using StdoutSink  = spdlog::sinks::stdout_color_sink_mt;
using SystemdSink = spdlog::sinks::systemd_sink_st;

template <typename SinkType>
auto createSink(bool enable) {
  auto sink = std::make_shared<SinkType>();
  sink->set_level(enable ? spdlog::level::trace : spdlog::level::off);
  return sink;
}

}  // namespace

namespace cvs::logger {

class DefaultLogger : public ILogger {
  friend DefaultLoggerFactory;

 public:
  DefaultLogger(std::shared_ptr<spdlog::logger> ptr)
      : ILogger(std::move(ptr)) {}

  std::string_view name() const override;
  LogImage         logImage() const override;

  Level                        level() const override;
  const std::filesystem::path& path() const override;

  bool isEnabled(Level) const override;

  std::filesystem::path p         = std::filesystem::temp_directory_path();
  LogImage              log_image = LogImage::disable;
};

std::string_view DefaultLogger::name() const { return logger->name(); }
LogImage         DefaultLogger::logImage() const { return log_image; }
Level            DefaultLogger::level() const { return convertLogLevel(logger->level()); }
const std::filesystem::path& DefaultLogger::path() const { return p; }
bool DefaultLogger::isEnabled(Level l) const { return logger->should_log(convertLogLevel(l)); }

}  // namespace cvs::logger

namespace cvs::logger {

void DefaultLoggerFactory::configureImpl(Regex ptrn, std::any val) {
  std::unique_lock lock(mutex);

  std::string re_ptrn{ptrn};

  if (val.type() == typeid(Level))
    config_cache[re_ptrn].level = std::any_cast<Level>(val);
  else if (val.type() == typeid(Pattern)) {
    auto p                          = std::any_cast<Pattern>(val);
    config_cache[re_ptrn].pattern   = p;
    config_cache[re_ptrn].time_type = p.time_type;
  } else if (val.type() == typeid(std::filesystem::path))
    config_cache[re_ptrn].path = std::any_cast<std::filesystem::path>(val);
  else if (val.type() == typeid(Sinks))
    config_cache[re_ptrn].sinks = std::any_cast<Sinks>(val);
  else if (val.type() == typeid(LogImage))
    config_cache[re_ptrn].log_image = std::any_cast<LogImage>(val);
}

void DefaultLoggerFactory::configureImpl() {
  std::shared_lock lock(mutex);

  for (auto& cfg : config_cache) {
    std::regex re(cfg.first);
    for (auto& logger : created_loggers) {
      if (std::regex_match(logger.first, re)) {
        configureLogger(logger.second, cfg.second);
      }
    }
  }
}

void DefaultLoggerFactory::configureLogger(const LoggerPtr& logger, const LogConf& config) const {
  auto def_logger = std::dynamic_pointer_cast<DefaultLogger>(logger);
  if (def_logger) {
    if (config.level)
      def_logger->logger->set_level(DefaultLogger::convertLogLevel(config.level.value()));
    if (config.pattern) {
      if (config.time_type)
        def_logger->logger->set_pattern(config.pattern.value(),
                                        convertTimeType(config.time_type.value()));
      else
        def_logger->logger->set_pattern(config.pattern.value());
    }

    def_logger->p = config.path;

    if (config.log_image)
      def_logger->log_image = config.log_image.value();

    if (config.sinks) {
      auto sinks_flags = config.sinks.value();
      auto sinks       = def_logger->logger->sinks();
      for (auto& s : sinks) {
        auto std_sink = std::dynamic_pointer_cast<StdoutSink>(s);
        if (std_sink) {
          std_sink->set_level(sinks_flags & Sinks::STDOUT ? spdlog::level::trace
                                                          : spdlog::level::off);
          continue;
        }
        auto sys_sink = std::dynamic_pointer_cast<SystemdSink>(s);
        if (sys_sink) {
          sys_sink->set_level(sinks_flags & Sinks::SYSTEMD ? spdlog::level::trace
                                                           : spdlog::level::off);
          continue;
        }
      }
    }
  }
}

LoggerPtr DefaultLoggerFactory::createLogger(std::string name) const {
  std::vector<spdlog::sink_ptr> sinks;
  sinks.push_back(createSink<StdoutSink>(default_sinks & Sinks::STDOUT));
  sinks.push_back(createSink<SystemdSink>(default_sinks & Sinks::SYSTEMD));

  auto logger = std::make_shared<spdlog::logger>(name, std::begin(sinks), std::end(sinks));
  if (name == default_logger_name)
    spdlog::set_default_logger(logger);
  else
    spdlog::register_logger(logger);

  return std::make_shared<DefaultLogger>(std::move(logger));
}

LoggerPtr DefaultLoggerFactory::getLoggerImpl(std::string_view n) {
  std::string name{n};
  {
    std::shared_lock lock(mutex);

    if (auto iter = created_loggers.find(name); iter != created_loggers.end())
      return iter->second;
  }

  LoggerPtr logger;
  {
    std::unique_lock lock(mutex);

    logger = createLogger(name);
    created_loggers.emplace(name, logger);
  }

  std::shared_lock lock(mutex);
  for (auto& cfg : config_cache) {
    std::regex re(cfg.first);
    if (std::regex_match(name, re))
      configureLogger(logger, cfg.second);
  }

  return logger;
}

LoggerPtr DefaultLoggerFactory::defaultLoggerImpl() { return getLoggerImpl(default_logger_name); }

std::string DefaultLoggerFactory::logNameToRegexPattern(std::string_view name) const {
  std::string name_pattern{name};
  for (auto& ch : "\\^.[$()|*+?{") {
    std::string::size_type pos = 0;
    do {
      pos = name_pattern.find(ch, pos);
      if (std::string::npos != pos) {
        name_pattern.replace(pos, 1, std::string({'\\', ch}));
        pos += 2;
      }
    } while (std::string::npos != pos);
  }

  return name_pattern;
}

}  // namespace cvs::logger
