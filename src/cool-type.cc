#include "cool-type.h"

#define INVALID_INDEX UINT_MAX

InheritanceTree::InheritanceTree(void) {
  /* Install basic classes */

  // Object:
  //  - abort() : Object
  //  - type_name() : String
  //  - copy() : SELF_TYPE
  dict.insert({ Symbol::Object, 0 });
  nodes.push_back(
    {
      Symbol::Object,
      INVALID_INDEX,
      0,
      {
        { Symbol::Object, { strtab.new_string("abort"), { Symbol::Object, {} } } },
        { Symbol::Object, { strtab.new_string("type_name"), { Symbol::String, {} } } },
        { Symbol::Object, { strtab.new_string("copy"), { Symbol::SELF_TYPE, {} } } },
      }
    });

  // IO:
  //  - out_string(x : String) : SELF_TYPE
  //  - out_int(x : Int) : SELF_TYPE
  //  - in_string() : String
  //  - in_int() : Int
  dict.insert({ Symbol::IO, 1 });
  nodes.push_back(
    {
      Symbol::IO,
      0,
      1,
      {
        { Symbol::IO, { strtab.new_string("out_string"), { Symbol::SELF_TYPE, { { strtab.new_string("x"), Symbol::String } } } } },
        { Symbol::IO, { strtab.new_string("out_int"), { Symbol::SELF_TYPE, { { strtab.new_string("x"), Symbol::Int } } } } },
        { Symbol::IO, { strtab.new_string("in_string"), { Symbol::String, {} } } },
        { Symbol::IO, { strtab.new_string("in_int"), { Symbol::Int, {} } } },
      }
    });

  // Int:
  dict.insert({ Symbol::Int, 2 });
  nodes.push_back(
    {
      Symbol::Int,
      0,
      1
    });

  // String:
  //  - length() : Int
  //  - concat(s : String) : String
  //  - substr(i : Int, l : Int) : String
  dict.insert({ Symbol::String, 3 });
  nodes.push_back(
    {
      Symbol::String,
      0,
      1,
      {
        { Symbol::String, { strtab.new_string("length"), { Symbol::Int, {} } } },
        { Symbol::String, { strtab.new_string("concat"), { Symbol::String, { { strtab.new_string("s"), Symbol::String } } } } },
        { Symbol::String, { strtab.new_string("substr"), { Symbol::String, { { strtab.new_string("i"), Symbol::Int }, { strtab.new_string("l"), Symbol::Int } } } } },
      }
    });

  // Bool:
  dict.insert({ Symbol::Bool, 4 });
  nodes.push_back(
    {
      Symbol::Bool,
      0,
      1
    });
}

bool InheritanceTree::isInheritable(Symbol *typeName) const {
  auto iter = dict.find(typeName);
  if (iter != dict.cend()) {
    if (2 <= iter->second && iter->second <= 4) {
      return false;
    }
  }
  return true;
}


bool InheritanceTree::isConform(Symbol *C, Symbol *T1, Symbol *T2) const {
  /**
   * SELF_TYPE{C} <= SELF_TYPE{C}
   * 
   * In Cool we never need to compare SELF_TYPEs coming from different
   * classes.
   */
  if (T1 == Symbol ::SELF_TYPE || T2 == Symbol ::SELF_TYPE) {
    return true;
  }

  /**
   * SELF_TYPE{C} <= T2 if C <= T2
   * 
   *  - SELF_TYPE{C} can be any subtype of C;
   *  - This includes C itself;
   *  - Thus this is the most flexible rule we can allow.
   */
  if (T1 == Symbol::SELF_TYPE) {
    return isConform(C, C, T2);
  }

  /**
   * T1 <= SELF_TYPE{C} always false
   * 
   * Note that SELF_TYPE{C} can denote any subtype of C.
   */
  if (T2 == Symbol::SELF_TYPE) {
    return false;
  }

  /**
   * T1 <= T2 if T1 is a subtype of T2
   */

  const Node *T2Node = &nodes[dict.find(T2)->second];
  const Node *T1Node = &nodes[dict.find(T1)->second];

  while (T1Node->depth > T2Node->depth) {
    T1Node = &nodes[T1Node->base_index];
  }

  return T1Node->name == T2;
}

Symbol *InheritanceTree::lub(Symbol *C, Symbol *T1, Symbol *T2) const {
  /* lub(SELF_TYPE{C}, SELF_TYPE{C}) = SELF_TYPE{C} */
  if (T1 == Symbol::SELF_TYPE && T2 == Symbol::SELF_TYPE) {
    return Symbol::SELF_TYPE;
  }

  /* lub(SELF_TYPE{C}, T2) = lub(C, T2) */
  if (T1 == Symbol::SELF_TYPE) {
    T1 = C;
  }

  /* lub(T1, SELF_TYPE{C}) = lub(T1, C) */
  if (T2 == Symbol::SELF_TYPE) {
    T2 = C;
  }

  /* lub(T1, T2) defined as before */

  const Node *T1Node = &nodes[dict.find(T1)->second];
  const Node *T2Node = &nodes[dict.find(T2)->second];

  while (T1Node->depth > T2Node->depth) {
    T1Node = &nodes[T1Node->base_index];
  }

  while (T2Node->depth > T1Node->depth) {
    T2Node = &nodes[T2Node->base_index];
  }

  while (T1Node != T2Node) {
    T1Node = &nodes[T1Node->base_index];
    T2Node = &nodes[T2Node->base_index];
  }

  return T1Node->name;
}

const AttributeInfo *InheritanceTree::getAttributeInfo(Symbol *typeName, Symbol *attrName) const {
  auto iter = dict.find(typeName);
  if (iter != dict.cend()) {
    unsigned int type_index = iter->second;
    while (type_index != -1) {
      const Node &node = nodes[type_index];
      auto iter = node.attrs.find(attrName);
      if (iter != node.attrs.cend()) {
        return &iter->second;
      }
      type_index = node.base_index;
    }
  }
  return nullptr;
}

const MethodInfo *InheritanceTree::getMethodInfo(Symbol *typeName, Symbol *methName) const {
  auto iter = dict.find(typeName);
  if (iter != dict.cend()) {
    unsigned int type_index = iter->second;
    while (type_index != -1) {
      const Node &node = nodes[type_index];
      auto iter = node.meths.find(methName);
      if (iter != node.meths.cend()) {
        return &iter->second;
      }
      type_index = node.base_index;
    }
  }
  return nullptr;
}

bool InheritanceTree::installClass(Symbol *name, Symbol *baseName) {
  if (dict.find(name) != dict.cend()) {
    return false;
  }

  auto iter = dict.find(baseName);
  if (iter == dict.cend()) {
    return false;
  }

  unsigned int base_index = iter->second;

  dict.insert({ name, nodes.size() });
  nodes.push_back({ name, base_index, nodes[base_index].depth + 1 });

  return true;
}

bool InheritanceTree::installAttribute(Symbol *typeName, Symbol *attrName, Symbol *attrType) {
  auto iter = dict.find(typeName);
  if (iter == dict.cend()) {
    return false;
  }
  return nodes[iter->second].attrs.insert({ attrName, { typeName, attrType } }).second;
}

bool InheritanceTree::installMethod(Symbol *typeName, Symbol *methName, Symbol *retType, const std::vector<std::pair<Symbol *, Symbol *>> &paramDecls) {
  auto iter = dict.find(typeName);
  if (iter == dict.cend()) {
    return false;
  }
  return nodes[iter->second].meths.insert({
    methName, {
      typeName, {
        retType, paramDecls
      }
    }
  }).second;
}
