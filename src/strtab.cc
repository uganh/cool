#include "strtab.h"

Strtab strtab;

Symbol *const Symbol::Bool      = strtab.new_string("Bool");
Symbol *const Symbol::IO        = strtab.new_string("IO");
Symbol *const Symbol::Int       = strtab.new_string("Int");
Symbol *const Symbol::Object    = strtab.new_string("Object");
Symbol *const Symbol::SELF_TYPE = strtab.new_string("SELF_TYPE");
Symbol *const Symbol::String    = strtab.new_string("String");
Symbol *const Symbol::self      = strtab.new_string("self");

Strtab::~Strtab(void) {
  for (const auto &item : dict) {
    delete item.second;
  }
  dict.clear();
}

Symbol *Strtab::new_string(const std::string &str) {
  auto iter = dict.find(str);
  if (iter == dict.cend()) {
    iter = dict.insert({ str, new Symbol(str) }).first;
  }
  return iter->second;
}
