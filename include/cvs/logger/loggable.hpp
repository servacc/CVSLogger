#pragma once

#include <boost/core/demangle.hpp>
#include <cvs/logger/logging.hpp>

#include <string_view>

namespace cvs::logger {

template <typename T>
class Loggable {
 public:
  explicit Loggable(const std::string& name = boost::core::demangle(typeid(T).name()),
                    std::optional<cvs::common::Config> cfg = std::nullopt) {
    log = createLogger(name, cfg);
  }
  virtual ~Loggable() = default;

  void configureLogger(cvs::common::Config& cfg) { cvs::logger::configureLogger(logger(), cfg); }

  [[nodiscard]] LoggerPtr&       logger() { return log; }
  [[nodiscard]] const LoggerPtr& logger() const { return log; }

 private:
  LoggerPtr log;
};

}  // namespace cvs::logger
