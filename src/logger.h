#ifndef COOL_LOGGER_H
#define COOL_LOGGER_H

#include <cstdlib>
#include <iostream>

class Logger {
  unsigned errors;
  std::ostream &stream;

public:
  Logger(std::ostream &stream) : errors(0), stream(stream) {}

  Logger(const Logger &) = delete;
  Logger &operator=(const Logger &) = delete;

  virtual ~Logger(void) noexcept = default;

  std::ostream &debug(const std::string &file, long line) {
    return stream << file << " at " << line << ": ";
  }

  std::ostream &warning(void) {
    return stream;
  }

  std::ostream &error(void) {
    ++errors;
    return stream << "\033[1;31merror\033[0m: ";
  }

  void checkpoint(void) const {
    if (errors) {
      stream << "Compilation halted" << std::endl;
      exit(EXIT_FAILURE);
    }
  }
};

#endif