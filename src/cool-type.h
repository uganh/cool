#ifndef COOL_TYPE_H
#define COOL_TYPE_H

#include <cassert>
#include <memory>
#include <string>
#include <unordered_map>
#include <vector>

class Type;

class FunctionType {
  friend class Type;

  const Type *return_type;
  std::vector<const Type *> parameter_types;

  FunctionType(
    const Type *return_type,
    const std::vector<const Type *> &parameter_types) :
    return_type(return_type), parameter_types(parameter_types) {}

  static FunctionType *create(
    const Type *return_type,
    const std::vector<const Type *> &parameter_types = {}) {
    return new FunctionType(return_type, parameter_types);
  }

public:
  const Type *getReturnType(void) const {
    return return_type;
  }

  const std::vector<const Type *> &getParameterTypes(void) const {
    return parameter_types;
  }
};

class Type {
  static const Type SELF_TYPE_private;
  static const Type Object_private;
  static const Type IO_private;
  static const Type String_private;
  static const Type Int_private;
  static const Type Bool_private;

  unsigned depth;
  std::string name;
  const Type *base;

  using AttrDict =
    std::unordered_map<std::string, std::pair<unsigned, const Type *>>;

  /* It is illegal to redefine attribute names. */
  std::shared_ptr<AttrDict> attrs;

  /*
   * In the case that a parent and child both define the same method name, then
   * the definition given in the child class takes precedence.
   * */

  /*
   * To ensure type safety, there are restrictions on the redefinition of
   * inherited methods. The rule is simple: If a class C inherits a method f
   * from an ancestor class P, then C may override the inherited definition of f
   * provided the number of arguments, the types of the formal parameters, and
   * the return type are exactly the same in both definitions.
   * */
  std::unordered_map<std::string, const FunctionType *> funcs;

  Type(
    const std::string &name,
    const Type *base,
    const std::unordered_map<std::string, const Type *> &attrs         = {},
    const std::unordered_map<std::string, const FunctionType *> &funcs = {}) :
    name(name),
    base(base),
    attrs(base ? base->attrs : std::make_shared<AttrDict>()),
    funcs(funcs),
    depth(base ? base->depth + 1 : 0) {}

public:
  static const Type *const SELF_TYPE;
  static const Type *const Object_type;
  static const Type *const IO_type;
  static const Type *const String_type;
  static const Type *const Int_type;
  static const Type *const Bool_type;

  static Type *create(const std::string &name, const Type *base) {
    return new Type(name, base);
  }

  static const Type *lub(const Type *C, const Type *T1, const Type *T2);
  static bool isConform(const Type *C, const Type *T1, const Type *T2);

  Type(const Type &) = delete;
  Type &operator=(const Type &) = delete;

  ~Type(void) noexcept {
    for (auto &func : funcs) {
      delete func.second;
    }
  }

  const std::string &getName(void) const {
    return name;
  }

  const Type *getParent(void) const {
    return base;
  }

  bool install(const std::string &name, const Type *attr_type) {
    return attrs->insert({name, std::make_pair(depth, attr_type)}).second;
  }

  bool install(
    const std::string &name,
    const std::pair<const Type *, std::vector<const Type *>> &prototype) {
    if (funcs.find(name) != funcs.cend()) {
      return false;
    }

    const FunctionType *base_func_type = base->getFunctionType(name);
    if (base_func_type) {
      if (
        base_func_type->return_type != prototype.first ||
        base_func_type->parameter_types != prototype.second) {
        return false;
      }
    }

    FunctionType *func_type =
      FunctionType::create(prototype.first, prototype.second);
    assert(funcs.insert({name, func_type}).second);

    return true;
  }

  const Type *getAttributeType(const std::string &name) const {
    auto iter = attrs->find(name);
    if (iter != attrs->cend()) {
      if (iter->second.first <= depth) {
        return iter->second.second;
      }
    }
    return nullptr;
  }

  const FunctionType *getFunctionType(const std::string &name) const {
    const Type *curr = this;
    while (curr != nullptr) {
      auto iter = curr->funcs.find(name);
      if (iter != curr->funcs.cend()) {
        return iter->second;
      }
      curr = curr->base;
    }
    return nullptr;
  }

  bool isConform(const Type *that) const {
    /* Warning: Internal used */
    /* Neither this nor that are SELF_TYPE */
    const Type *curr = this;
    while (curr->depth > that->depth) {
      curr = curr->base;
    }
    return curr == that;
  }
};

#endif