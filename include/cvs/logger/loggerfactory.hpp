#pragma once

#include <any>
#include <filesystem>
#include <functional>
#include <memory>

#include <cvs/logger/configtypes.hpp>
#include <cvs/logger/cvslogger_export.hpp>

namespace cvs::logger {

using LoggerPtr       = std::shared_ptr<class ILogger>;
using LoggerFactorPtr = std::shared_ptr<class LoggerFactory>;

class CVSLOGGER_EXPORT LoggerFactory {
 public:
  static const std::string_view default_logger_name;

  static LoggerFactorPtr instance();
  static void            registerCreator(std::function<LoggerFactorPtr()>);

  static LoggerPtr getLogger(std::string_view name) {
    return instance()->getLoggerImpl(std::move(name));
  }
  static LoggerPtr defaultLogger() { return instance()->defaultLoggerImpl(); }

  static void configure() { instance()->configureImpl(); }
  template <typename Name, typename... Args, typename... Loggers>
  static void configure(Name name, std::tuple<Args...> args, Loggers... loggers) {
    std::apply(
        [&](auto... x) {
          (static_cast<void>(instance()->configureImpl(name, std::make_any<decltype(x)>(x))), ...);
        },
        args);
    configure(loggers...);
  }

 public:
  virtual ~LoggerFactory() = default;

 protected:
  void configureImpl(std::string_view, std::any);

  virtual void configureImpl(Regex, std::any) = 0;
  virtual void configureImpl()                = 0;

  virtual LoggerPtr getLoggerImpl(std::string_view) = 0;
  virtual LoggerPtr defaultLoggerImpl()             = 0;

  virtual std::string logNameToRegexPattern(std::string_view) const = 0;

 private:
  static std::function<LoggerFactorPtr()> creator;
};

}  // namespace cvs::logger
