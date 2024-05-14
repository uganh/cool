#include "cool-type.h"

#include <stack>

#define INVALID_INDEX UINT_MAX

InheritanceTree::InheritanceTree(void) : fixed(false) {
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
      new ClassInfo {
        Symbol::Object, // typeName
        nullptr,        // base
        true,           // isPrimitive
        true,           // inheritable
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

  ClassInfo *root = nodes[0].classInfo;

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
      new ClassInfo {
        Symbol::IO, // typeName
        root,       // base
        true,       // isPrimitive
        true,       // inheritable
        0,          // wordSize
        root->dispatchTable,
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
      new ClassInfo {
        Symbol::Int, // typeName
        root,        // base
        true,        // isPrimitive
        false,       // inheritable
        1,           // wordSize
        root->dispatchTable,
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
      new ClassInfo {
        Symbol::String, // typeName
        root,           // base
        true,           // isPrimitive
        false,          // inheritable
        2,              // wordSize
        root->dispatchTable,
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
      new ClassInfo {
        Symbol::Bool, // typeName
        root,         // base
        true,         // isPrimitive
        false,        // inheritable
        1,            // wordSize
        root->dispatchTable,
      }
    });
}

InheritanceTree::~InheritanceTree(void) {
  for (Node &node : nodes) {
    for (auto &item : node.classInfo->attributes) {
      delete item.second;
    }

    for (auto &item : node.classInfo->methods) {
      delete item.second;
    }

    delete node.classInfo;
  }
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

  return T1Node->classInfo->typeName == T2;
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

  return T1Node->classInfo->typeName;
}

const AttributeInfo *InheritanceTree::getAttributeInfo(Symbol *typeName, Symbol *attrName) const {
  auto iter = dict.find(typeName);
  if (iter != dict.cend()) {
    unsigned int type_index = iter->second;
    while (type_index != -1) {
      const Node &node = nodes[type_index];
      auto iter = node.classInfo->attributes.find(attrName);
      if (iter != node.classInfo->attributes.cend()) {
        return iter->second;
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
      auto iter = node.classInfo->methods.find(methName);
      if (iter != node.classInfo->methods.cend()) {
        return iter->second;
      }
      type_index = node.base_index;
    }
  }
  return nullptr;
}

const ClassInfo *InheritanceTree::getClassInfo(Symbol *typeName) const {
  auto iter = dict.find(typeName);
  if (iter != dict.cend()) {
    return nodes[iter->second].classInfo;
  }
  return nullptr;
}

bool InheritanceTree::installClass(Symbol *name, Symbol *baseName) {
  if (fixed) {
    return false;
  }

  if (dict.find(name) != dict.cend()) {
    return false;
  }

  auto iter = dict.find(baseName);
  if (iter == dict.cend()) {
    return false;
  }

  unsigned int index = static_cast<unsigned int>(nodes.size());
  unsigned int base_index = iter->second;

  Node &baseNode = nodes[base_index];
  ClassInfo *base = baseNode.classInfo;

  dict.insert({ name, index });
  nodes.push_back(
    {
      base_index,
      baseNode.depth + 1,
      new ClassInfo {
        name,           // typeName
        base,           // base
        false,          // isPrimitive
        true,           // inheritable
        base->wordSize, // wordSize
        base->dispatchTable,
      }
    });

  return true;
}

bool InheritanceTree::installAttribute(Symbol *typeName, Symbol *attrName, Symbol *attrType, Expression *init) {
  auto iter = dict.find(typeName);
  if (iter == dict.cend()) {
    return false;
  }

  Node &node = nodes[iter->second];
  ClassInfo *classInfo = node.classInfo;

  auto insertion = classInfo->attributes.insert(
    {
      attrName,
      new AttributeInfo {
        typeName,
        attrType,
        init,
        classInfo->wordSize
      }
    });

  if (insertion.second) {
    classInfo->wordSize++;
    return true;
  }

  return false;
}

bool InheritanceTree::installMethod(Symbol *typeName, Symbol *methName, Symbol *retType, const std::vector<std::pair<Symbol *, Symbol *>> &paramDecls, Expression *expr) {
  auto iter = dict.find(typeName);
  if (iter == dict.cend()) {
    return false;
  }

  Node &node = nodes[iter->second];
  ClassInfo *classInfo = node.classInfo;

  const MethodInfo *baseMethodInfo = getMethodInfo(typeName, methName);
  unsigned int index = baseMethodInfo ?
    baseMethodInfo->index :
    static_cast<unsigned int>(classInfo->dispatchTable.size());

  auto insertion = classInfo->methods.insert({
    methName,
    new MethodInfo {
      typeName,
      methName,
      { retType, paramDecls },
      expr,
      index
    },
  });

  if (insertion.second) {
    MethodInfo *methodInfo = insertion.first->second;
    if (baseMethodInfo) {
      classInfo->dispatchTable[index] = methodInfo;
    } else {
      classInfo->dispatchTable.push_back(methodInfo);
    }
    return true;
  }

  return false;
}

void InheritanceTree::fix(void) {
  std::unordered_map<ClassInfo *, std::vector<ClassInfo *>> graph;

  for (Node &node : nodes) {
    ClassInfo *classInfo = node.classInfo;
    ClassInfo *baseClassInfo = classInfo->base;
    if (baseClassInfo) {
      graph[baseClassInfo].push_back(classInfo);
    }
  }

  unsigned int tag = 0;
  std::stack<std::pair<ClassInfo *, bool>> stack;
  stack.push({ nodes[0].classInfo, false} );

  while (!stack.empty()) {
    auto &item = stack.top();
    ClassInfo *classInfo = item.first;
    if (item.second) {
      classInfo->tagEnd = tag;
      stack.pop();
    }
    else {
      classInfo->tag = tag++;
      const std::vector<ClassInfo *> edges = graph[classInfo];
      for (auto iter = edges.rbegin(), last = edges.rend(); iter != last; iter++) {
        stack.push({ *iter, false });
      }
      item.second = true;
    }
  }

  fixed = true;
}
