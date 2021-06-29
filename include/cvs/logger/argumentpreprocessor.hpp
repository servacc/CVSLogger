#pragma once

#include <spdlog/spdlog.h>

namespace cvs::logger {

template <typename T>
struct ArgumentPreprocessor {
  template <typename Arg>
  static auto exec(const std::shared_ptr<spdlog::logger>&, spdlog::level::level_enum, Arg&& arg) {
    return std::forward<Arg>(arg);
  }
};

}  // namespace cvs::logger
