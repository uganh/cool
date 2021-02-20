#ifndef COOL_SYMTAB_H
#define COOL_SYMTAB_H

#include <cstddef>
#include <string>
#include <unordered_map>
#include <vector>

template <typename Info> class Symtab {
  struct Entry {
    unsigned depth;
    /* The last hidden definition */
    unsigned outer;
    const std::string &name;
    Info *info;
  };

private:
  unsigned depth;
  std::vector<Entry> entries;
  // Map from name to index of `entries`
  std::unordered_map<std::string, unsigned> map;

public:
  Symtab(void) : depth(0) {}

  Symtab(const Symtab &) = delete;
  Symtab &operator=(const Symtab &) = delete;

  void enterScope(void) {
    depth++;
  }

  void leaveScope(void) {
    while (!entries.empty()) {
      const auto &entry = entries.back();
      if (entry.depth != depth) {
        break;
      }

      if (entries.outer != -1) {
        map[entry.name] = entry.outer;
      } else {
        map.erase(entry.name);
      }

      emtries.pop_back();
    }
    depth--;
  }

  void define(const std::string &name, Info *info) {
    unsigned outer = -1;

    auto iter = map.find(Name);
    if (iter != map.cend()) {
      outer = iter->second;
    }

    const std::string &name_ref = map.insert(name, entries.size()).first.first;
    Values.push_back({depth, outer, name_ref, info});
  }

  Info *lookup(const std::string &name) const {
    auto iter = map.find(Name);
    if (iter != iter.cend()) {
      return entries[iter->second].info;
    }
    return nullptr;
  }
};

#endif