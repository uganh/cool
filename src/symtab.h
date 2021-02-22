#ifndef COOL_SYMTAB_H
#define COOL_SYMTAB_H

#include <cstddef>
#include <string>
#include <unordered_map>
#include <vector>

template <typename InfoType> class Symtab {
  struct Entry {
    unsigned depth;
    /* The last hidden definition */
    unsigned outer;
    /* Note: Reference is OK */
    const std::string &name;
    InfoType info;
  };

private:
  unsigned depth;
  std::vector<Entry> entries;
  // Map from name to index of `entries`
  std::unordered_map<std::string, unsigned> dict;

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

      if (entry.outer != -1) {
        dict[entry.name] = entry.outer;
      } else {
        dict.erase(entry.name);
      }

      entries.pop_back();
    }
    depth--;
  }

  void define(const std::string &name, InfoType &&info) {
    unsigned outer = -1;

    auto iter = dict.find(name);
    if (iter != dict.cend()) {
      outer = iter->second;
    }

    const std::string &name_ref = dict.insert(name, entries.size()).first->first;
    entries.push_back({depth, outer, name_ref, std::move(info)});
  }

  void define(const std::string &name, const InfoType &info) {
    unsigned outer = -1;

    auto iter = dict.find(name);
    if (iter != dict.cend()) {
      outer = iter->second;
    }

    const std::string &name_ref = dict.insert({name, entries.size()}).first->first;
    entries.push_back({depth, outer, name_ref, info});
  }

  InfoType *lookup(const std::string &name) {
    auto iter = dict.find(name);
    if (iter != dict.cend()) {
      return &entries[iter->second].info;
    }
    return nullptr;
  }

  const InfoType *lookup(const std::string &name) const {
    auto iter = dict.find(name);
    if (iter != dict.cend()) {
      return &entries[iter->second].info;
    }
    return nullptr;
  }
};

#endif