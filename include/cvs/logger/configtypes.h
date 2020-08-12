#pragma once

#include <memory>
#include <string>

#include <cvs/logger/cvslogger_export.h>

namespace cvs::logger {

enum class Sinks : int {
  NOSINK  = 0,
  STDOUT  = 1,
  SYSTEMD = 2,
};

constexpr bool  operator&(Sinks s0, Sinks s1) { return (int(s0) & int(s1)) != 0; }
constexpr Sinks operator|(Sinks s0, Sinks s1) { return Sinks{int(s0) | int(s1)}; }

enum class Level {
  trace    = 0,
  debug    = 1,
  info     = 2,
  warn     = 3,
  err      = 4,
  critical = 5,
  off      = 6,
};

enum class TimeType { local = 0, utc };

enum class LogImage { disable = 0, enable };

class CVSLOGGER_EXPORT Regex : public std::string_view {
 public:
  constexpr explicit Regex(const std::string_view& other) noexcept
      : std::string_view(other) {}
  constexpr explicit Regex(const char* s, size_type count)
      : std::string_view(s, count) {}
  constexpr explicit Regex(const char* s)
      : std::string_view(s) {}
};

class CVSLOGGER_EXPORT Pattern : public std::string_view {
 public:
  constexpr explicit Pattern(const std::string_view& other, TimeType tt = TimeType::local) noexcept
      : std::string_view(other)
      , time_type(tt) {}
  constexpr explicit Pattern(const char* s, size_type count, TimeType tt = TimeType::local)
      : std::string_view(s, count)
      , time_type(tt) {}
  constexpr explicit Pattern(const char* s, TimeType tt = TimeType::local)
      : std::string_view(s)
      , time_type(tt) {}

  const TimeType time_type;
};

}  // namespace cvs::logger
