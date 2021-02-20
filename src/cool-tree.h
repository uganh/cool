#ifndef COOL_TREE_H
#define COOL_TREE_H

#include <string>
#include <vector>

#include "strtab.h"
#include "utilities.h"

class TreeNode {
  /* TODO */
  unsigned lineno;

public:
  TreeNode(void)             = default;
  TreeNode(const TreeNode &) = delete;
  TreeNode &operator=(const TreeNode &) = delete;
  virtual ~TreeNode(void) noexcept      = default;

  virtual void dump(std::ostream &stream, unsigned indent) const = 0;
};

class Feature : public TreeNode {
public:
  virtual ~Feature(void) noexcept = default;
};

class Expression : public TreeNode {
public:
  virtual ~Expression(void) noexcept = default;
};

class Parameter : TreeNode {
  Symbol name;
  Symbol type;

public:
  Parameter(Symbol name, Symbol type) : name(name), type(type) {}

  void dump(std::ostream &stream, unsigned indent) const override {
    stream << pad(indent) << "Parameter " << *name << "@" << *type << std::endl;
  }
};

class Class : public TreeNode {
  Symbol name;
  Symbol base;
  std::vector<std::unique_ptr<Feature>> features;

public:
  Class(
    Symbol name,
    Symbol base,
    std::vector<std::unique_ptr<Feature>> &&features) :
    name(name), base(base), features(std::move(features)) {}

  void dump(std::ostream &stream, unsigned indent) const override {
    stream << pad(indent) << "Class " << *name << ":" << *base << std::endl;
    for (const auto &feature : features) {
      feature->dump(stream, indent + 1);
    }
  }
};

class Program : public TreeNode {
  std::vector<std::unique_ptr<Class>> classes;

public:
  explicit Program(std::vector<std::unique_ptr<Class>> &&classes) :
    classes(std::move(classes)) {}

  void dump(std::ostream &stream, unsigned indent = 0) const override {
    stream << pad(indent) << "Program" << std::endl;
    for (const auto &claSs : classes) {
      claSs->dump(stream, indent + 1);
    }
  }
};

class Method : public Feature {
  Symbol name;
  Symbol type;
  std::vector<std::unique_ptr<Parameter>> params;
  std::unique_ptr<Expression> body;

public:
  Method(
    Symbol name,
    Symbol type,
    std::vector<std::unique_ptr<Parameter>> &&params,
    std::unique_ptr<Expression> &&body) :
    name(name), type(type), params(std::move(params)), body(std::move(body)) {}

  void dump(std::ostream &stream, unsigned indent) const override {
    stream << pad(indent) << "Method " << *name << "@" << *type << std::endl;
    for (const auto &param : params) {
      param->dump(stream, indent + 1);
    }
    body->dump(stream, indent + 1);
  }
};

class Attribute : public Feature {
  Symbol name;
  Symbol type;
  std::unique_ptr<Expression> init;

public:
  Attribute(
    Symbol name, Symbol type, std::unique_ptr<Expression> &&init = nullptr) :
    name(name), type(type), init(std::move(init)) {}

  void dump(std::ostream &stream, unsigned indent) const override {
    stream << pad(indent) << "Attribute " << name << ":" << type << std::endl;
    init->dump(stream, indent + 1);
  }
};

class Assign : public Expression {
  Symbol name;
  std::unique_ptr<Expression> expr;

public:
  Assign(Symbol name, std::unique_ptr<Expression> &&expr) :
    name(name), expr(std::move(expr)) {}

  void dump(std::ostream &stream, unsigned indent) const override {
    stream << pad(indent) << "Assign " << *name << std::endl;
    expr->dump(stream, indent + 1);
  }
};

class StaticDispatch : public Expression {
  std::unique_ptr<Expression> expr;
  Symbol type;
  Symbol name;
  std::vector<std::unique_ptr<Expression>> args;

public:
  StaticDispatch(
    std::unique_ptr<Expression> &&expr,
    Symbol type,
    Symbol name,
    std::vector<std::unique_ptr<Expression>> &&args) :
    expr(std::move(expr)), type(type), name(name), args(std::move(args)) {}

  void dump(std::ostream &stream, unsigned indent) const override {
    stream << pad(indent) << "StaticDispatch @" << *type << ", " << *name
           << std::endl;
    expr->dump(stream, indent + 1);
    for (const auto &arg : args) {
      arg->dump(stream, indent + 1);
    }
  }
};

class Dispatch : public Expression {
  std::unique_ptr<Expression> expr;
  Symbol name;
  std::vector<std::unique_ptr<Expression>> args;

public:
  Dispatch(
    std::unique_ptr<Expression> &&expr,
    Symbol name,
    std::vector<std::unique_ptr<Expression>> &&args) :
    expr(std::move(expr)), name(name), args(std::move(args)) {}

  void dump(std::ostream &stream, unsigned indent) const override {
    stream << pad(indent) << "Dispatch " << *name << std::endl;
    expr->dump(stream, indent + 1);
    for (const auto &arg : args) {
      arg->dump(stream, indent + 1);
    }
  }
};

class Conditional : public Expression {
  std::unique_ptr<Expression> pred, then, Else;

public:
  Conditional(
    std::unique_ptr<Expression> &&pred,
    std::unique_ptr<Expression> &&then,
    std::unique_ptr<Expression> &&Else) :
    pred(std::move(pred)), then(std::move(then)), Else(std::move(Else)) {}

  void dump(std::ostream &stream, unsigned indent) const override {
    stream << pad(indent) << "Conditional" << std::endl;
    pred->dump(stream, indent + 1);
    then->dump(stream, indent + 1);
    Else->dump(stream, indent + 1);
  }
};

class Loop : public Expression {
  std::unique_ptr<Expression> pred, body;

public:
  Loop(std::unique_ptr<Expression> &&pred, std::unique_ptr<Expression> &&body) :
    pred(std::move(pred)), body(std::move(body)) {}

  void dump(std::ostream &stream, unsigned indent) const override {
    stream << pad(indent) << "Loop" << std::endl;
    pred->dump(stream, indent + 1);
    body->dump(stream, indent + 1);
  }
};

class Block : public Expression {
  std::vector<std::unique_ptr<Expression>> body;

public:
  Block(std::vector<std::unique_ptr<Expression>> &&body) :
    body{std::move(body)} {}

  void dump(std::ostream &stream, unsigned indent) const override {
    stream << pad(indent) << "Block" << std::endl;
    for (const auto &expr : body) {
      expr->dump(stream, indent + 1);
    }
  }
};

class Definition : public TreeNode {
  Symbol name;
  Symbol type;
  std::unique_ptr<Expression> init;

public:
  Definition(
    Symbol name, Symbol type, std::unique_ptr<Expression> &&init = nullptr) :
    name(name), type(type), init(std::move(init)) {}

  void dump(std::ostream &stream, unsigned indent) const override {
    stream << pad(indent) << "Definition " << *name << "@" << *type
           << std::endl;
    init->dump(stream, indent + 1);
  }
};

class Let : public Expression {
  std::vector<std::unique_ptr<Definition>> defs;
  std::unique_ptr<Expression> body;

public:
  Let(
    std::vector<std::unique_ptr<Definition>> &&defs,
    std::unique_ptr<Expression> &&body) :
    defs(std::move(defs)), body(std::move(body)) {}

  void dump(std::ostream &stream, unsigned indent) const override {
    stream << pad(indent) << "Let" << std::endl;
    for (const auto &def : defs) {
      def->dump(stream, indent + 1);
    }
    body->dump(stream, indent + 1);
  }
};

class CaseBranch : TreeNode {
  Symbol name;
  Symbol type;
  std::unique_ptr<Expression> expr;

public:
  CaseBranch(Symbol name, Symbol type, std::unique_ptr<Expression> &&expr) :
    name(name), type(type), expr(std::move(expr)) {}

  void dump(std::ostream &stream, unsigned indent) const override {
    stream << pad(indent) << "CaseBranch " << *name << "@" << *type
           << std::endl;
    expr->dump(stream, indent + 1);
  }
};

class Case : public Expression {
  std::unique_ptr<Expression> expr;
  std::vector<std::unique_ptr<CaseBranch>> cases;

public:
  Case(
    std::unique_ptr<Expression> &&expr,
    std::vector<std::unique_ptr<CaseBranch>> &&cases) :
    expr(std::move(expr)), cases(std::move(cases)) {}

  void dump(std::ostream &stream, unsigned indent) const override {
    stream << pad(indent) << "Case" << std::endl;
    expr->dump(stream, indent + 1);
    for (const auto &cAse : cases) {
      cAse->dump(stream, indent + 1);
    }
  }
};

class Binary : public Expression {
public:
  enum Ops { ADD, SUB, MUL, DIV, LT, EQ, LE };

private:
  Ops opcode;
  std::unique_ptr<Expression> op1, op2;

public:
  Binary(
    Ops opcode,
    std::unique_ptr<Expression> &&op1,
    std::unique_ptr<Expression> &&op2) :
    opcode(opcode), op1(std::move(op1)), op2(std::move(op2)) {}

  void dump(std::ostream &stream, unsigned indent) const override {
    static const std::string opstr[]{"+", "-", "*", "/", "<", "=", "<="};
    stream << pad(indent) << "Binary " << opstr[opcode] << std::endl;
    op1->dump(stream, indent + 1);
    op2->dump(stream, indent + 1);
  }
};

class Unary : public Expression {
public:
  enum Ops { NEG, NOT };

private:
  Ops opcode;
  std::unique_ptr<Expression> expr;

public:
  Unary(Ops opcode, std::unique_ptr<Expression> &&expr) :
    opcode(opcode), expr(std::move(expr)) {}

  void dump(std::ostream &stream, unsigned indent) const override {
    static const std::string opstr[] = {"~", "not"};
    stream << pad(indent) << "Unary " << opstr[opcode] << std::endl;
    expr->dump(stream, indent + 1);
  }
};

class Boolean : public Expression {
  bool value;

public:
  Boolean(bool value) : value(value) {}

  void dump(std::ostream &stream, unsigned indent) const override {
    stream << pad(indent) << "Boolean " << value << std::endl;
  }
};

class Integer : public Expression {
  long value;

public:
  Integer(long value) : value(value) {}

  void dump(std::ostream &stream, unsigned indent) const override {
    stream << pad(indent) << "Integer " << value << std::endl;
  }
};

class String : public Expression {
  std::string value;

public:
  String(const std::string &value) : value(value) {}

  void dump(std::ostream &stream, unsigned indent) const override {
    stream << pad(indent) << "String \"" << escaped_string(value) << "\""
           << std::endl;
  }
};

class Object : public Expression {
  Symbol name;

public:
  Object(Symbol name) : name(name) {}

  void dump(std::ostream &stream, unsigned indent) const override {
    stream << pad(indent) << "Object " << *name << std::endl;
  }
};

class New : public Expression {
  Symbol type;

public:
  New(Symbol type) : type(type) {}

  void dump(std::ostream &stream, unsigned indent) const override {
    stream << pad(indent) << "New @" << *type << std::endl;
  }
};

class Isvoid : public Expression {
  std::unique_ptr<Expression> expr;

public:
  Isvoid(std::unique_ptr<Expression> &&expr) : expr(std::move(expr)) {}

  void dump(std::ostream &stream, unsigned indent) const override {
    stream << pad(indent) << "Isvoid" << std::endl;
    expr->dump(stream, indent + 1);
  }
};

#endif