#pragma once

#include "cool-type.h"
#include "symtab.h"

#include <ostream>
#include <string>
#include <vector>

using ScopedSymtab = Symtab<Symbol>;

using ScopedSymtabGuard = SymtabGuard<Symbol>;

class Program;

class TreeNode {
public:
  virtual ~TreeNode(void) = default;

  virtual void dump(std::ostream &stream, std::vector<bool> &indents, const Program *program) const = 0;
};

class Expression : public TreeNode {
public:
  virtual Symbol *typeCheck(
    InheritanceTree &inheritanceTree,
    const Program *program,
    Symbol *currentType,
    ScopedSymtab &symtab) const = 0;
};

class Assign : public Expression {
  Symbol *left; // Cannot be self
  Expression *expr;

public:
  Assign(Symbol *left, Expression *expr) : left(left), expr(expr) {}

  virtual void dump(std::ostream &stream, std::vector<bool> &indents, const Program *program) const override;

  virtual Symbol *typeCheck(
    InheritanceTree &inheritanceTree,
    const Program *program,
    Symbol *currentType,
    ScopedSymtab &symtab) const override;
};

class Dispatch : public Expression {
  Expression *expr;
  Symbol *name;
  Symbol *type; // Cannot be SELF_TYPE
  std::vector<Expression *> args;

public:
  Dispatch(Expression *expr, Symbol *name, Symbol *type, std::vector<Expression *> args)
    : expr(expr), name(name), type(type), args(std::move(args)) {}
  
  virtual void dump(std::ostream &stream, std::vector<bool> &indents, const Program *program) const override;

  virtual Symbol *typeCheck(
    InheritanceTree &inheritanceTree,
    const Program *program,
    Symbol *currentType,
    ScopedSymtab &symtab) const override;
};

class Conditional : public Expression {
  Expression *pred;
  Expression *then;
  Expression *elSe;

public:
  Conditional(Expression *pred, Expression *then, Expression *elSe)
    : pred(pred), then(then), elSe(elSe) {}

  virtual void dump(std::ostream &stream, std::vector<bool> &indents, const Program *program) const override;

  virtual Symbol *typeCheck(
    InheritanceTree &inheritanceTree,
    const Program *program,
    Symbol *currentType,
    ScopedSymtab &symtab) const override;
};

class Loop : public Expression {
  Expression *pred;
  Expression *body;

public:
  Loop(Expression *pred, Expression *body) : pred(pred), body(body) {}

  virtual void dump(std::ostream &stream, std::vector<bool> &indents, const Program *program) const override;

  virtual Symbol *typeCheck(
    InheritanceTree &inheritanceTree,
    const Program *program,
    Symbol *currentType,
    ScopedSymtab &symtab) const override;
};

class Block : public Expression {
  std::vector<Expression *> exprs;

public:
  explicit Block(std::vector<Expression *> exprs) : exprs(std::move(exprs)) {}

  virtual void dump(std::ostream &stream, std::vector<bool> &indents, const Program *program) const override;

  virtual Symbol *typeCheck(
    InheritanceTree &inheritanceTree,
    const Program *program,
    Symbol *currentType,
    ScopedSymtab &symtab) const override;
};

class Definition : public TreeNode {
  Symbol *name; // Cannot be self
  Symbol *type;
  Expression *init;

public:
  Definition(Symbol *name, Symbol *type, Expression *init = nullptr)
    : name(name), type(type), init(init) {}

  virtual void dump(std::ostream &stream, std::vector<bool> &indents, const Program *program) const override;

  void install(
    InheritanceTree &inheritanceTree,
    const Program *program,
    Symbol *currentType,
    ScopedSymtab &symtab) const;
};

class Let : public Expression {
  std::vector<Definition *> defs;
  Expression *body;

public:
  Let(std::vector<Definition *> defs, Expression *body)
    : defs(std::move(defs)), body(body) {}

  virtual void dump(std::ostream &stream, std::vector<bool> &indents, const Program *program) const override;

  virtual Symbol *typeCheck(
    InheritanceTree &inheritanceTree,
    const Program *program,
    Symbol *currentType,
    ScopedSymtab &symtab) const override;
};

class Branch : public TreeNode {
  Symbol *name; // Cannot be self
  Symbol *type; // Cannot be SELF_TYPE
  Expression *expr;

public:
  Branch(Symbol *name, Symbol *type, Expression *expr)
    : name(name), type(type), expr(expr) {}

  virtual void dump(std::ostream &stream, std::vector<bool> &indents, const Program *program) const override;

  std::pair<Symbol *, Symbol *> doCheck(
    InheritanceTree &inheritanceTree,
    const Program *program,
    Symbol *currentType,
    ScopedSymtab &symtab) const;
};

class Case : public Expression {
  Expression *expr;
  std::vector<Branch *> branches;

public:
  Case(Expression *expr, std::vector<Branch *> branches)
    : expr(expr), branches(std::move(branches)) {}

  virtual void dump(std::ostream &stream, std::vector<bool> &indents, const Program *program) const override;

  virtual Symbol *typeCheck(
    InheritanceTree &inheritanceTree,
    const Program *program,
    Symbol *currentType,
    ScopedSymtab &symtab) const override;
};

class New : public Expression {
  Symbol *type;

public:
  explicit New(Symbol *type) : type(type) {}

  virtual void dump(std::ostream &stream, std::vector<bool> &indents, const Program *program) const override;

  virtual Symbol *typeCheck(
    InheritanceTree &inheritanceTree,
    const Program *program,
    Symbol *currentType,
    ScopedSymtab &symtab) const override;
};

class IsVoid : public Expression {
  Expression *expr;

public:
  explicit IsVoid(Expression *expr) : expr(expr) {}

  virtual void dump(std::ostream &stream, std::vector<bool> &indents, const Program *program) const override;

  virtual Symbol *typeCheck(
    InheritanceTree &inheritanceTree,
    const Program *program,
    Symbol *currentType,
    ScopedSymtab &symtab) const override;
};

enum class ArithmeticOperator { ADD, SUB, MUL, DIV, };

class Arithmetic : public Expression {
  ArithmeticOperator op;
  Expression *op1;
  Expression *op2;

public:
  Arithmetic(ArithmeticOperator op, Expression *op1, Expression *op2)
    : op(op), op1(op1), op2(op2) {}

  virtual void dump(std::ostream &stream, std::vector<bool> &indents, const Program *program) const override;

  virtual Symbol *typeCheck(
    InheritanceTree &inheritanceTree,
    const Program *program,
    Symbol *currentType,
    ScopedSymtab &symtab) const override;
};

class Complement : public Expression {
  Expression *expr;

public:
  explicit Complement(Expression *expr) : expr(expr) {}

  virtual void dump(std::ostream &stream, std::vector<bool> &indents, const Program *program) const override;

  virtual Symbol *typeCheck(
    InheritanceTree &inheritanceTree,
    const Program *program,
    Symbol *currentType,
    ScopedSymtab &symtab) const override;
};

enum class ComparisonOperator { LT, LE, EQ, };

class Comparison : public Expression {
  ComparisonOperator op;
  Expression *op1;
  Expression *op2;

public:
  Comparison(ComparisonOperator op, Expression *op1, Expression *op2)
    : op(op), op1(op1), op2(op2) {}

  virtual void dump(std::ostream &stream, std::vector<bool> &indents, const Program *program) const override;

  virtual Symbol *typeCheck(
    InheritanceTree &inheritanceTree,
    const Program *program,
    Symbol *currentType,
    ScopedSymtab &symtab) const override;
};

class Not : public Expression {
  Expression *expr;

public:
  explicit Not(Expression *expr) : expr(expr) {}

  virtual void dump(std::ostream &stream, std::vector<bool> &indents, const Program *program) const override;

  virtual Symbol *typeCheck(
    InheritanceTree &inheritanceTree,
    const Program *program,
    Symbol *currentType,
    ScopedSymtab &symtab) const override;
};

class Object : public Expression {
  Symbol *name;

public:
  explicit Object(Symbol *name) : name(name) {}

  virtual void dump(std::ostream &stream, std::vector<bool> &indents, const Program *program) const override;

  virtual Symbol *typeCheck(
    InheritanceTree &inheritanceTree,
    const Program *program,
    Symbol *currentType,
    ScopedSymtab &symtab) const override;
};

class Integer : public Expression {
  long value;

public:
  explicit Integer(long value) : value(value) {}

  virtual void dump(std::ostream &stream, std::vector<bool> &indents, const Program *program) const override;

  virtual Symbol *typeCheck(
    InheritanceTree &inheritanceTree,
    const Program *program,
    Symbol *currentType,
    ScopedSymtab &symtab) const override;
};

class String : public Expression {
  std::string value;

public:
  explicit String(const std::string &value) : value(value) {}

  virtual void dump(std::ostream &stream, std::vector<bool> &indents, const Program *program) const override;

  virtual Symbol *typeCheck(
    InheritanceTree &inheritanceTree,
    const Program *program,
    Symbol *currentType,
    ScopedSymtab &symtab) const override;
};

class Boolean : public Expression {
  bool value;

public:
  explicit Boolean(bool value) : value(value) {}

  virtual void dump(std::ostream &stream, std::vector<bool> &indents, const Program *program) const override;

  virtual Symbol *typeCheck(
    InheritanceTree &inheritanceTree,
    const Program *program,
    Symbol *currentType,
    ScopedSymtab &symtab) const override;
};

class Feature : public TreeNode {
public:
  virtual void install(
    InheritanceTree &inheritanceTree,
    const Program *program,
    Symbol *currentType) const = 0;

  virtual void doCheck(
    InheritanceTree &inheritanceTree,
    const Program *program,
    Symbol *currentType,
    ScopedSymtab &symtab) const = 0;
};

class Attribute : public Feature {
  Symbol *name;
  Symbol *type;
  Expression *init;

public:
  Attribute(Symbol *name, Symbol *type, Expression *init = nullptr)
    : name(name), type(type), init(init) {}

  virtual void dump(std::ostream &stream, std::vector<bool> &indents, const Program *program) const override;

  virtual void install(
    InheritanceTree &inheritanceTree,
    const Program *program,
    Symbol *currentType) const override;

  virtual void doCheck(
    InheritanceTree &inheritanceTree,
    const Program *program,
    Symbol *currentType,
    ScopedSymtab &symtab) const override;
};

class Formal : public TreeNode {
  Symbol *name; // Cannot be self
  Symbol *type; // Cannot be SELF_TYPE

public:
  Formal(Symbol *name, Symbol *type) : name(name), type(type) {}

  Symbol *getName(void) const {
    return name;
  }

  Symbol *getType(void) const {
    return type;
  }

  virtual void dump(std::ostream &stream, std::vector<bool> &indents, const Program *program) const override;
};

class Method : public Feature {
  Symbol *name;
  std::vector<Formal *> formals;
  Symbol *type;
  Expression *expr;

public:
  Method(Symbol *name, std::vector<Formal *> formals, Symbol *type, Expression *expr)
    : name(name), formals(std::move(formals)), type(type), expr(expr) {}

  virtual void dump(std::ostream &stream, std::vector<bool> &indents, const Program *program) const override;

  virtual void install(
    InheritanceTree &inheritanceTree,
    const Program *program,
    Symbol *currentType) const override;

  virtual void doCheck(
    InheritanceTree &inheritanceTree,
    const Program *program,
    Symbol *currentType,
    ScopedSymtab &symtab) const override;
};

class Class : public TreeNode {
  Symbol *name;
  Symbol *base;
  std::vector<Feature *> features;

public:
  Class(Symbol *name, Symbol *base, std::vector<Feature *> features)
    : name(name), base(base), features(std::move(features)) {}

  Symbol *getName(void) const {
    return name;
  }

  Symbol *getBaseName(void) const {
    return base;
  }

  virtual void dump(std::ostream &stream, std::vector<bool> &indents, const Program *program) const override;

  void install(InheritanceTree &inheritanceTree) const;

  void doCheck(InheritanceTree &inheritanceTree, const Program *program) const;
};

class Program {
  std::string name;
  std::vector<Class *> classes;

  std::unordered_map<const TreeNode *, unsigned int> nodes;

public:
  explicit Program(const std::string &name) : name(name) {}

  ~Program(void) noexcept;

  const std::string &getName(void) const {
    return name;
  }

  const std::vector<Class *> getClasses(void) const {
    return classes;
  }

  unsigned int getLine(const TreeNode *node) const {
    return nodes.find(node)->second;
  }

  void addClass(Class *claSs) {
    classes.push_back(claSs);
  }

  void dump(std::ostream &stream) const;

  void doCheck(InheritanceTree &inheritanceTree) const;

  template <typename NodeType, typename ...Args>
  NodeType *new_tree_node(unsigned int line, Args &&...args) {
    NodeType *node = new NodeType(std::forward<Args>(args)...);
    nodes.insert({ node, line });
    return node;
  }
};
