#include "../include/cvs/logger/logging.hpp"

#include <spdlog/sinks/stdout_color_sinks.h>
#include <spdlog/sinks/systemd_sink.h>
#include <spdlog/spdlog.h>

#include <cvs/common/configbase.hpp>
#include <cvs/common/staticfactory.hpp>

using namespace cvs::logger;
using namespace std::string_literals;

namespace {

CVSCFG_DECLARE_CONFIG(LoggerConfig,
                      CVSCFG_VALUE(name, std::string),
                      CVSCFG_VALUE_OPTIONAL(level, int),
                      CVSCFG_VALUE_OPTIONAL(pattern, std::string),
                      CVSCFG_VALUE_OPTIONAL(time_type, int),
                      CVSCFG_VALUE_OPTIONAL(sink, int))

using StdoutSink  = spdlog::sinks::stdout_color_sink_mt;
using SystemdSink = spdlog::sinks::systemd_sink_st;

enum class Sinks : int {
  NOSINK  = 0,
  STDOUT  = 1,
  SYSTEMD = 2,
};

constexpr bool  operator&(Sinks s0, Sinks s1) { return (int(s0) & int(s1)) != 0; }
constexpr Sinks operator|(Sinks s0, Sinks s1) { return Sinks{int(s0) | int(s1)}; }

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

cvs::logger::LoggerPtr createLogger(std::string name) {
  std::vector<spdlog::sink_ptr> sinks;
  sinks.push_back(createSink<StdoutSink>(default_sinks & Sinks::STDOUT));
  sinks.push_back(createSink<SystemdSink>(default_sinks & Sinks::SYSTEMD));

  return std::make_shared<spdlog::logger>(name, std::begin(sinks), std::end(sinks));
}

cvs::logger::LoggerPtr getOrCreateLogger(std::string name) {
  auto logger = spdlog::get(name);
  if (!logger) {
    logger = createLogger(name);
    spdlog::register_logger(logger);
  }
  return logger;
}

void configureLogger(cvs::logger::LoggerPtr& logger, LoggerConfig& config) {
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

void configureLogger(LoggerPtr logger, cvs::common::Config& cfg) {
  auto cfg_struct_opt = cfg.parse<LoggerConfig>();
  if (cfg_struct_opt)
    configureLogger(logger, *cfg_struct_opt);
}

LoggerPtr createLogger(std::string name, std::optional<common::Config> config) {
  if (config)
    return common::StaticFactory::create<LoggerPtr>("cvslogger"s, name, *config).value();
  return common::StaticFactory::create<LoggerPtr>("cvslogger"s, name).value();
}

void createDefaultLogger(std::optional<cvs::common::Config> config) {
  auto default_logger = createLogger("");
  spdlog::set_default_logger(default_logger);

  if (config) {
    if (auto c = config->parse<LoggerConfig>(); c)
      configureLogger(default_logger, *c);
  }
}

void initLoggers(std::optional<common::Config> config) {
  registerLoggersInFactory();
  createDefaultLogger(std::nullopt);

  if (config) {
    auto loggers = config->getChildren();
    for (auto c : loggers) {
      auto logger_conf = c.parse<LoggerConfig>();
      if (logger_conf) {
        auto logger = getOrCreateLogger(logger_conf->name);
        configureLogger(logger, *logger_conf);
      }
    }
  }
}

void registerLoggersInFactory() {
  common::StaticFactory::registrate<LoggerPtr(std::string)>("cvslogger"s, getOrCreateLogger);

  common::StaticFactory::registrate<LoggerPtr(cvs::common::Config)>(
      "cvslogger"s, [](cvs::common::Config cfg) -> LoggerPtr {
        auto cfg_struct_opt = cfg.parse<LoggerConfig>();
        if (cfg_struct_opt) {
          auto logger_opt =
              common::StaticFactory::create<LoggerPtr>("cvslogger"s, cfg_struct_opt->name);
          if (!logger_opt)
            return {};
          configureLogger(*logger_opt, *cfg_struct_opt);
          return *logger_opt;
        }

        return {};
      });
}

}  // namespace cvs::logger
