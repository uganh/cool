#ifndef COOL_SYMBOL_H
#define COOL_SYMBOL_H

#include <string>
#include <iostream>

class Symbol {
  friend struct std::hash<Symbol>;
  friend bool operator==(const Symbol &lhs, const Symbol &rhs);
  friend std::ostream &operator<<(std::ostream &, const Symbol &);

  const std::string &name;

public:
  /* FIXME: Bison need a default constructor */
  Symbol(const std::string &name = "") : name(name) {}

  Symbol(const Symbol &that) : name(that.name) {}
  Symbol(Symbol &&that) : name(that.name) {}

  Symbol &operator=(const Symbol &) = delete;
  Symbol &operator=(Symbol &&) = delete;

  const std::string &str(void) const {
    return name;
  }
};

template <> struct std::hash<Symbol> {
  size_t operator()(const Symbol &symbol) const {
    return std::hash<std::string>()(symbol.name);
  }
};

inline bool operator==(const Symbol &lhs, const Symbol &rhs) {
  return lhs.name == rhs.name;
}

inline std::ostream &operator<<(std::ostream &stream, const Symbol &symbol) {
  return stream << symbol.name;
}

#endif