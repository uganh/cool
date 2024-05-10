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
      INVALID_INDEX,    // base_index
      0,                // depth
      {                 // classInfo
        Symbol::Object, // typeName
        0,              // index
        0,              // wordSize
      }
    });
  installMethod(
    Symbol::Object,
    strtab.new_string("abort"),
    Symbol::Object,
    {},
    nullptr);
  installMethod(
    Symbol::Object,
    strtab.new_string("type_name"),
    Symbol::String,
    {},
    nullptr);
  installMethod(
    Symbol::Object,
    strtab.new_string("copy"),
    Symbol::SELF_TYPE,
    {},
    nullptr);

  // IO:
  //  - out_string(x : String) : SELF_TYPE
  //  - out_int(x : Int) : SELF_TYPE
  //  - in_string() : String
  //  - in_int() : Int
  dict.insert({ Symbol::IO, 1 });
  nodes.push_back(
    {
      0,            // base_index
      1,            // depth
      {             // classInfo
        Symbol::IO, // typeName
        1,          // index
        0,          // wordSize
      }
    });
  installMethod(
    Symbol::IO,
    strtab.new_string("out_string"),
    Symbol::SELF_TYPE,
    {
      { strtab.new_string("x"), Symbol::String }
    },
    nullptr);
  installMethod(
    Symbol::IO,
    strtab.new_string("out_int"),
    Symbol::SELF_TYPE,
    {
      { strtab.new_string("x"), Symbol::Int }
    },
    nullptr);
  installMethod(
    Symbol::IO,
    strtab.new_string("in_string"),
    Symbol::String,
    {},
    nullptr);
  installMethod(
    Symbol::IO,
    strtab.new_string("in_int"),
    Symbol::Int,
    {},
    nullptr);

  // Int:
  dict.insert({ Symbol::Int, 2 });
  nodes.push_back(
    {
      0,             // base_index
      1,             // depth
      {              // classInfo
        Symbol::Int, // typeName
        2,           // index
        1,           // wordSize
      }
    });

  // String:
  //  - length() : Int
  //  - concat(s : String) : String
  //  - substr(i : Int, l : Int) : String
  dict.insert({ Symbol::String, 3 });
  nodes.push_back(
    {
      0,                // base_index
      1,                // depth
      {                 // classInfo
        Symbol::String, // typeName
        3,              // index
        0,              // wordSize
      }
    });
  installMethod(
    Symbol::String,
    strtab.new_string("length"),
    Symbol::Int,
    {},
    nullptr);
  installMethod(
    Symbol::String,
    strtab.new_string("concat"),
    Symbol::String,
    {
      { strtab.new_string("s"), Symbol::String }
    },
    nullptr);
  installMethod(
    Symbol::String,
    strtab.new_string("substr"),
    Symbol::String,
    {
      { strtab.new_string("i"), Symbol::Int },
      { strtab.new_string("l"), Symbol::Int }
    },
    nullptr);

  // Bool:
  dict.insert({ Symbol::Bool, 4 });
  nodes.push_back(
    {
      0,              // base_index
      1,              // depth
      {               // classInfo
        Symbol::Bool, // typeName
        4,            // index
        0,            // wordSize
      }
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

  return T1Node->classInfo.typeName == T2;
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

  return T1Node->classInfo.typeName;
}

const AttributeInfo *InheritanceTree::getAttributeInfo(Symbol *typeName, Symbol *attrName) const {
  auto iter = dict.find(typeName);
  if (iter != dict.cend()) {
    unsigned int type_index = iter->second;
    while (type_index != -1) {
      const Node &node = nodes[type_index];
      auto iter = node.classInfo.attributes.find(attrName);
      if (iter != node.classInfo.attributes.cend()) {
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
      auto iter = node.classInfo.methods.find(methName);
      if (iter != node.classInfo.methods.cend()) {
        return &iter->second;
      }
      type_index = node.base_index;
    }
  }
  return nullptr;
}

const ClassInfo *InheritanceTree::getClassInfo(Symbol *typeName) const {
  auto iter = dict.find(typeName);
  if (iter != dict.cend()) {
    return &nodes[iter->second].classInfo;
  }
  return nullptr;
}

bool InheritanceTree::installClass(Symbol *name, Symbol *baseName) {
  if (dict.find(name) != dict.cend()) {
    return false;
  }

  unsigned int index = nodes.size();
  unsigned int depth = 0;
  unsigned int base_index = INVALID_INDEX;

  auto iter = dict.find(baseName);
  if (iter != dict.cend()) {
    base_index = iter->second;
    depth = nodes[base_index].depth + 1;
  }

  dict.insert({ name, index });
  nodes.push_back({ base_index, depth, { name, index } });

  if (base_index != INVALID_INDEX) {
    ClassInfo &classInfo = nodes.back().classInfo;
    ClassInfo &baseClassInfo = nodes[base_index].classInfo;
    classInfo.wordSize = baseClassInfo.wordSize;
    classInfo.dispatchTable = baseClassInfo.dispatchTable;
  }

  return true;
}

bool InheritanceTree::installAttribute(Symbol *typeName, Symbol *attrName, Symbol *attrType, Expression *init) {
  auto iter = dict.find(typeName);
  if (iter == dict.cend()) {
    return false;
  }

  ClassInfo &classInfo = nodes[iter->second].classInfo;

  if (classInfo.attributes.insert({ attrName, { typeName, attrType, init, classInfo.wordSize } }).second) {
    classInfo.wordSize++;
    return true;
  }

  return false;
}

bool InheritanceTree::installMethod(Symbol *typeName, Symbol *methName, Symbol *retType, const std::vector<std::pair<Symbol *, Symbol *>> &paramDecls, Expression *expr) {
  auto iter = dict.find(typeName);
  if (iter == dict.cend()) {
    return false;
  }

  ClassInfo &classInfo = nodes[iter->second].classInfo;

  unsigned int index = INVALID_INDEX;
  if (const MethodInfo *methInfo = getMethodInfo(typeName, methName)) {
    index = methInfo->index;
  }

  auto insertion = classInfo.methods.insert({
    methName,
    {
      typeName,
      { retType, paramDecls },
      expr,
      index
    },
  });

  if (insertion.second) {
    if (index != INVALID_INDEX) {
      classInfo.dispatchTable[index] = &insertion.first->second;
    } else {
      classInfo.dispatchTable.push_back(&insertion.first->second);
    }
    return true;
  }

  return false;
}
