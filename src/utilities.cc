#include <cctype>
#include <iomanip>
#include <iostream>
#include <sstream>

#include "utilities.h"

const char *pad(unsigned n) {
  static std::string spaces(80, ' ');
  if (n >= 80) {
    return spaces.c_str();
  } else {
    return spaces.c_str() + 80 - n;
  }
}

std::string escaped_string(const std::string &str) {
  std::ostringstream oss;
  for (auto c : str) {
    switch (c) {
      case '\\':
        /* Fall through */
      case '\"':
        oss << '\\' << c;
        break;
      case '\n':
        oss << "\\n";
        break;
      case '\t':
        oss << "\\t";
        break;
      case '\b':
        oss << "\\b";
        break;
      case '\f':
        oss << "\\f";
        break;
      default: {
        if (std::isprint(c)) {
          oss << std::string{c};
        } else {
          oss << '\\' << std::oct << std::setfill('0') << std::setw(2)
              << static_cast<int>(static_cast<unsigned char>(c)) << std::dec
              << std::setfill(' ');
        }
      }
    }
  }
  return oss.str();
}