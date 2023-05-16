#pragma once

#include "strtab.h"

#include <unordered_map>
#include <vector>

struct MethodInfo {
  Symbol *typeName;
  struct MethodType {
    Symbol *retType;
    std::vector<std::pair<Symbol *, Symbol *>> paramDecls;
  } methType;
};

struct AttributeInfo {
  Symbol *typeName;
  Symbol *attrType;
};

class InheritanceTree {
  struct Node {
    Symbol *name;
    unsigned int base_index;
    unsigned int depth;
    std::unordered_map<Symbol *, MethodInfo> meths;
    std::unordered_map<Symbol *, AttributeInfo> attrs;
  };

  std::vector<Node> nodes;
  std::unordered_map<Symbol *, unsigned int> dict;

public:
  InheritanceTree(void);

  bool isDefined(Symbol *typeName) const {
    return dict.find(typeName) != dict.cend();
  }

  bool isInheritable(Symbol * typeName) const;

  bool isConform(Symbol *C, Symbol *T1, Symbol *T2) const;

  /**
   * @brief The least-upper bound of T1 and T2
   * 
   * @param C 
   * @param T1 
   * @param T2 
   * @return Symbol* 
   */
  Symbol *lub(Symbol *C, Symbol *T1, Symbol *T2) const;

  const AttributeInfo *getAttributeInfo(Symbol *typeName, Symbol *attrName) const;

  const MethodInfo *getMethodInfo(Symbol *typeName, Symbol *methName) const;

  bool installClass(Symbol *name, Symbol *baseName);

  bool installAttribute(Symbol *typeName, Symbol *attrName, Symbol *attrType);

  bool installMethod(Symbol *typeName, Symbol *methName, Symbol *retType, const std::vector<std::pair<Symbol *, Symbol *>> &paramDecls);
};