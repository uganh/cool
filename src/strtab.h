#ifndef COOL_STRTAB_H
#define COOL_STRTAB_H

#include <string>
#include <unordered_set>

#include "symbol.h"

class Strtab {
  std::unordered_set<std::string> set;

public:
  Strtab(void) = default;

  Strtab(const Strtab &) = delete;
  Strtab &operator=(const Strtab &) = delete;

  Symbol get(const std::string &str) {
    /* Note: The pointer never fail */
    return Symbol(*set.insert(str).first);
  }
};

#endif