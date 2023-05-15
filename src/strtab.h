#pragma once

#include <string>
#include <unordered_map>

class Symbol {
  const std::string str;

public:
  static Symbol *const Bool;
  static Symbol *const IO;
  static Symbol *const Int;
  static Symbol *const Object;
  static Symbol *const SELF_TYPE;
  static Symbol *const String;
  static Symbol *const self;

  explicit Symbol(const std::string &str) : str(str) {}

  const std::string &to_string(void) const {
    return str;
  }
};

class Strtab {
  std::unordered_map<std::string, Symbol *> dict;

public:
  Strtab(void) = default;

  Strtab(const Strtab &) = delete;
  Strtab &operator=(const Strtab &) = delete;

  ~Strtab(void);

  Symbol *new_string(const std::string &str);
};

extern Strtab strtab;
