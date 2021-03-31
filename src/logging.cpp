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

enum Sink {
  NO_SINK = 0,
  STDOUT  = 1,
  SYSTEMD = 2,
};

const std::string name_in_factory = "cvs.logger";

constexpr auto default_sinks =
#ifdef CVSLOGGER_STD_ENABLED
#ifdef CVSLOGGER_SYSD_ENABLED
    Sink::SYSTEMD | Sink::STDOUT;
#else
    Sink::STDOUT;
#endif
#else
#ifdef CVSLOGGER_SYSD_ENABLED
    Sink::SYSTEMD;
#else
    Sink::NO_SINK;
#endif
#endif

template <typename SinkType>
auto createSink(bool enable_by_default) {
  auto sink = std::make_shared<SinkType>();
  if (!enable_by_default)
    sink->set_level(spdlog::level::off);
  return sink;
}

cvs::logger::LoggerPtr createLogger(const std::string& name) {
  return std::make_shared<spdlog::logger>(
      name, spdlog::sinks_init_list{createSink<StdoutSink>(default_sinks & Sink::STDOUT),
                                    createSink<SystemdSink>(default_sinks & Sink::SYSTEMD)});
}

cvs::logger::LoggerPtr getOrCreateLogger(const std::string& name) {
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
    auto sinks_flags = Sink(*config.sink);
    auto sinks       = logger->sinks();
    for (auto& s : sinks) {
      auto std_sink = std::dynamic_pointer_cast<StdoutSink>(s);
      if (std_sink) {
        std_sink->set_level(sinks_flags & Sink::STDOUT ? level : spdlog::level::off);
        continue;
      }
      auto sys_sink = std::dynamic_pointer_cast<SystemdSink>(s);
      if (sys_sink) {
        sys_sink->set_level(sinks_flags & Sink::SYSTEMD ? level : spdlog::level::off);
        continue;
      }
    }
  }
}

}  // namespace

namespace cvs::logger {

bool configureLogger(LoggerPtr logger, cvs::common::Config& cfg) {
  auto cfg_struct_opt = cfg.parse<LoggerConfig>();
  if (cfg_struct_opt) {
    configureLogger(logger, *cfg_struct_opt);
    return true;
  }

  return false;
}

LoggerPtr createLogger(const std::string& name, std::optional<common::Config> config) {
  if (config)
    return common::StaticFactory::create<LoggerPtr, std::string, const std::string&>(
               name_in_factory, name, *config)
        .value();
  return common::StaticFactory::create<LoggerPtr, std::string, const std::string&>(name_in_factory,
                                                                                   name)
      .value();
}

void createDefaultLogger(std::optional<cvs::common::Config> config) {
  auto default_logger = createLogger("");
  spdlog::set_default_logger(default_logger);

  if (config) {
    if (auto c = config->parse<LoggerConfig>(); c)
      configureLogger(default_logger, *c);
  }
}

bool initLoggers(std::optional<common::Config> config) {
  registerLoggersInFactory();
  createDefaultLogger(std::nullopt);

  bool result = true;
  if (config) {
    auto loggers_list = config->getChildren("loggers");
    for (auto& iter : loggers_list) {
      auto loggers = iter.getChildren();
      for (auto channel_conf : loggers) {
        auto logger_conf = channel_conf.parse<LoggerConfig>();
        if (logger_conf) {
          auto logger = getOrCreateLogger(logger_conf->name);
          configureLogger(logger, *logger_conf);
        } else
          result = false;
      }
    }
  }

  return result;
}

void registerLoggersInFactory() {
  common::StaticFactory::registrate<LoggerPtr(const std::string&)>(name_in_factory,
                                                                   getOrCreateLogger);

  common::StaticFactory::registrate<LoggerPtr(cvs::common::Config&)>(
      name_in_factory, [](cvs::common::Config& cfg) -> LoggerPtr {
        auto cfg_struct_opt = cfg.parse<LoggerConfig>();
        if (cfg_struct_opt) {
          auto logger_opt =
              common::StaticFactory::create<LoggerPtr, std::string, const std::string&>(
                  name_in_factory, cfg_struct_opt->name);
          if (!logger_opt)
            return {};
          configureLogger(*logger_opt, *cfg_struct_opt);
          return *logger_opt;
        }

        return {};
      });
}

}  // namespace cvs::logger
