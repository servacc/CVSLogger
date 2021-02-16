#include "../include/cvs/logger/loggerfactory.h"

#include "../include/cvs/logger/ilogger.h"
#include "default/defaultfactory.h"

#include <spdlog/spdlog.h>

#include <functional>

using namespace cvs::logger;

namespace cvs::logger {

const std::string_view LoggerFactory::default_logger_name;

std::function<LoggerFactorPtr()> LoggerFactory::creator = []() {
  static auto factory = std::make_shared<DefaultLoggerFactory>();
  return factory;
};

LoggerFactorPtr LoggerFactory::instance() { return creator(); }

void LoggerFactory::registerCreator(std::function<LoggerFactorPtr()> new_creator) {
  creator = std::move(new_creator);
}

void LoggerFactory::configureImpl(std::string_view name, std::any val) {
  configureImpl(Regex(logNameToRegexPattern(name)), std::move(val));
}

}  // namespace cvs::logger
