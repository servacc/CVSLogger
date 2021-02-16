#pragma once

#include <cvs/logger/loggerfactory.hpp>

#include <map>
#include <optional>
#include <shared_mutex>

namespace cvs::logger {

class DefaultLoggerFactory : public LoggerFactory {
  struct LogConf {
    std::optional<Level>       level;
    std::optional<std::string> pattern;
    std::optional<TimeType>    time_type;
    std::filesystem::path      path = std::filesystem::temp_directory_path();
    std::optional<Sinks>       sinks;
    std::optional<LogImage>    log_image;
  };

 protected:
  void configureImpl(Regex, std::any) override;
  void configureImpl() override;

  LoggerPtr getLoggerImpl(std::string_view) override;
  LoggerPtr defaultLoggerImpl() override;

  std::string logNameToRegexPattern(std::string_view) const override;

 protected:
  virtual LoggerPtr createLogger(std::string) const;
  virtual void      configureLogger(const LoggerPtr& logger, const LogConf& config) const;

 private:
  std::map<std::string, LogConf>             config_cache;
  std::unordered_map<std::string, LoggerPtr> created_loggers;
  std::shared_mutex                          mutex;
};

}  // namespace cvs::logger
