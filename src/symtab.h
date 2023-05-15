#pragma once

#include "strtab.h"

#include <unordered_map>
#include <vector>

template <typename DataType>
class Symtab {
  struct Entry {
    unsigned int depth;
    /* The last hidden definition */
    unsigned int outer;
    Symbol *name;
    DataType *info;
  };

  unsigned int depth;
  std::vector<Entry> entries;
  /* Mapping from name to index of `entries` */
  std::unordered_map<Symbol *, unsigned int> dict;

public:
  Symtab(void) : depth(0) {}

  Symtab(const Symtab &) = delete;
  Symtab &operator=(const Symtab &) = delete;

  void enterScope(void) {
    ++depth;
  }

  void leaveScope(void) {
    while (!entries.empty()) {
      const auto &entry = entries.back();
      if (entry.depth != depth) {
        break;
      }

      if (entry.outer != -1) {
        dict[entry.name] = entry.outer;
      } else {
        dict.erase(entry.name);
      }

      entries.pop_back();
    }

    --depth;
  }

  bool define(Symbol *name, DataType *info, bool probe = false) {
    unsigned int outer = -1;

    auto iter = dict.find(name);
    if (iter != dict.cend()) {
      unsigned int index = iter->second;
      if (probe && entries[index].depth == depth) {
        return false;
      }
      outer = index;
    }

    dict[name] = static_cast<unsigned int>(entries.size());
    entries.push_back({ depth, outer, name, info });

    return true;
  }

  DataType *lookup(Symbol *name) const {
    auto iter = dict.find(name);
    if (iter != dict.cend()) {
      return entries[iter->second].info;
    }
    return nullptr;
  }
};

template <typename DataType>
class SymtabGuard {
  Symtab<DataType> &symtab;

public:
  SymtabGuard(Symtab<DataType> &symtab) : symtab(symtab) {
    symtab.enterScope();
  }

  ~SymtabGuard(void) {
    symtab.leaveScope();
  }
};
