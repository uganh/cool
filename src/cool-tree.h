#ifndef COOL_TREE_H
#define COOL_TREE_H

#include <string>
#include <vector>

#include "cool-type.h"
#include "logger.h"
#include "symtab.h"
#include "utilities.h"

using TypeDict = std::unordered_map<std::string, const Type *>;

class TreeNode {
  /* TODO */
  unsigned lineno;

public:
  TreeNode(void) = default;

  TreeNode(const TreeNode &) = delete;
  TreeNode &operator=(const TreeNode &) = delete;

  virtual ~TreeNode(void) noexcept = default;

  virtual void dump(std::ostream &stream, unsigned indent) const = 0;
};

class Feature : public TreeNode {
public:
  virtual ~Feature(void) noexcept = default;

  virtual void
  semant(Logger &logger, const TypeDict &types, Type *curr_type) const = 0;

  virtual void check(
    Logger &logger,
    TypeDict &types,
    Symtab<const Type *> &symtab,
    const Type *curr_type) const = 0;
};

class Expression : public TreeNode {
protected:
  const Type *etype;

public:
  Expression(void) : etype(nullptr) {}

  virtual ~Expression(void) noexcept = default;

  virtual const Type *check(
    Logger &logger,
    const TypeDict &types,
    Symtab<const Type *> &symtab,
    const Type *curr_type) = 0;

  std::string suffix(void) const {
    if (etype) {
      return " : @" + etype->getName();
    } else {
      return "";
    }
  }
};

class Attribute : public Feature {
  std::string name;
  std::string type;
  std::unique_ptr<Expression> init;

public:
  Attribute(
    std::string name,
    std::string type,
    std::unique_ptr<Expression> &&init = nullptr) :
    name(name), type(type), init(std::move(init)) {}

  void dump(std::ostream &stream, unsigned indent) const override {
    stream << pad(indent) << "Attribute " << name << " : @" << type;
    if (init) {
      stream << " {" << std::endl;
      init->dump(stream, indent + 1);
      stream << pad(indent) << "}";
    }
    stream << std::endl;
  }

  void
  semant(Logger &logger, const TypeDict &types, Type *curr_type) const override;

  void check(
    Logger &logger,
    TypeDict &types,
    Symtab<const Type *> &symtab,
    const Type *curr_type) const override;
};

class Method : public Feature {
  std::string name;
  std::string type;
  std::vector<std::pair<std::string, std::string>> params;
  std::unique_ptr<Expression> body;

public:
  Method(
    std::string name,
    std::string type,
    std::vector<std::pair<std::string, std::string>> &&params = {},
    std::unique_ptr<Expression> &&body                        = nullptr) :
    name(name), type(type), params(std::move(params)), body(std::move(body)) {}

  void dump(std::ostream &stream, unsigned indent) const override {
    stream << pad(indent) << "Method " << name << " => @" << type << " (";
    bool first = true;
    for (const auto &param : params) {
      if (!first) {
        stream << ", ";
      }
      stream << param.first << " : @" << param.second;
      first = false;
    }
    stream << ") {" << std::endl;
    body->dump(stream, indent + 1);
    stream << pad(indent) << "}" << std::endl;
  }

  void
  semant(Logger &logger, const TypeDict &types, Type *curr_type) const override;

  void check(
    Logger &logger,
    TypeDict &types,
    Symtab<const Type *> &symtab,
    const Type *curr_type) const override;
};

class Class : public TreeNode {
  std::string name;
  std::string base;
  std::vector<std::unique_ptr<Feature>> features;
  Type *type;

public:
  Class(
    std::string name,
    std::string base,
    std::vector<std::unique_ptr<Feature>> &&features = {}) :
    name(name), base(base), features(std::move(features)), type(nullptr) {}

  ~Class(void) noexcept {
    if (type) {
      delete type;
    }
  }

  const std::string &getName(void) const {
    return name;
  }

  const std::string &getParentName(void) const {
    return base;
  }

  void dump(std::ostream &stream, unsigned indent) const override {
    stream << pad(indent) << "Class " << name << " inherits " << base << " {"
           << std::endl;
    for (const auto &feature : features) {
      feature->dump(stream, indent + 1);
    }
    stream << pad(indent) << "}" << std::endl;
  }

  void semant(Logger &logger, TypeDict &types);

  void check(Logger &logger, TypeDict &types) const;
};

class Program : public TreeNode {
  std::vector<std::unique_ptr<Class>> classes;

public:
  explicit Program(std::vector<std::unique_ptr<Class>> &&classes) :
    classes(std::move(classes)) {}

  void dump(std::ostream &stream, unsigned indent = 0) const override {
    stream << pad(indent) << "Program {" << std::endl;
    for (const auto &claSs : classes) {
      claSs->dump(stream, indent + 1);
    }
    stream << pad(indent) << "}" << std::endl;
  }

  void semant(Logger &logger) const;
};

class Boolean : public Expression {
  bool value;

public:
  Boolean(bool value) : value(value) {}

  void dump(std::ostream &stream, unsigned indent) const override {
    stream << pad(indent) << "[Boolean " << (value ? "true" : "false") << "]"
           << suffix() << std::endl;
  }

  const Type *check(
    Logger &logger,
    const TypeDict &types,
    Symtab<const Type *> &symtab,
    const Type *curr_type) override;
};

class Integer : public Expression {
  long value;

public:
  Integer(long value) : value(value) {}

  void dump(std::ostream &stream, unsigned indent) const override {
    stream << pad(indent) << "[Integer " << value << "]" << suffix()
           << std::endl;
  }

  const Type *check(
    Logger &logger,
    const TypeDict &types,
    Symtab<const Type *> &symtab,
    const Type *curr_type) override;
};

class String : public Expression {
  std::string value;

public:
  String(const std::string &value) : value(value) {}

  void dump(std::ostream &stream, unsigned indent) const override {
    stream << pad(indent) << "[String \"" << escaped_string(value) << "\"]"
           << suffix() << std::endl;
  }

  const Type *check(
    Logger &logger,
    const TypeDict &types,
    Symtab<const Type *> &symtab,
    const Type *curr_type) override;
};

class Isvoid : public Expression {
  std::unique_ptr<Expression> expr;

public:
  Isvoid(std::unique_ptr<Expression> &&expr) : expr(std::move(expr)) {}

  void dump(std::ostream &stream, unsigned indent) const override {
    stream << pad(indent) << "Isvoid {" << std::endl;
    expr->dump(stream, indent + 1);
    stream << pad(indent) << "}" << suffix() << std::endl;
  }

  const Type *check(
    Logger &logger,
    const TypeDict &types,
    Symtab<const Type *> &symtab,
    const Type *curr_type) override;
};

class Not : public Expression {
  std::unique_ptr<Expression> expr;

public:
  Not(std::unique_ptr<Expression> &&expr) : expr(std::move(expr)) {}

  void dump(std::ostream &stream, unsigned indent) const override {
    stream << pad(indent) << "Not {" << std::endl;
    expr->dump(stream, indent + 1);
    stream << pad(indent) << "}" << suffix() << std::endl;
  }

  const Type *check(
    Logger &logger,
    const TypeDict &types,
    Symtab<const Type *> &symtab,
    const Type *curr_type) override;
};

class Negate : public Expression {
  std::unique_ptr<Expression> expr;

public:
  Negate(std::unique_ptr<Expression> &&expr) : expr(std::move(expr)) {}

  void dump(std::ostream &stream, unsigned indent) const override {
    stream << pad(indent) << "Negate {" << std::endl;
    expr->dump(stream, indent + 1);
    stream << pad(indent) << "}" << suffix() << std::endl;
  }

  const Type *check(
    Logger &logger,
    const TypeDict &types,
    Symtab<const Type *> &symtab,
    const Type *curr_type) override;
};

class Logical : public Expression {
public:
  enum Ops { LT, EQ, LE };

private:
  Ops op;
  std::unique_ptr<Expression> op1, op2;

public:
  Logical(
    Ops op,
    std::unique_ptr<Expression> &&op1,
    std::unique_ptr<Expression> &&op2) :
    op(op), op1(std::move(op1)), op2(std::move(op2)) {}

  void dump(std::ostream &stream, unsigned indent) const override {
    stream << pad(indent) << "Logical " << opstr() << " {" << std::endl;
    op1->dump(stream, indent + 1);
    op2->dump(stream, indent + 1);
    stream << pad(indent) << "}" << suffix() << std::endl;
  }

  const Type *check(
    Logger &logger,
    const TypeDict &types,
    Symtab<const Type *> &symtab,
    const Type *curr_type) override;

  std::string opstr(void) const {
    switch (op) {
      case LT:
        return "<";
      case EQ:
        return "=";
      case LE:
        return "<=";
      default:
        assert(0 && "Invalid logical operator");
    }
  }
};

class Arithmetic : public Expression {
  char op;
  std::unique_ptr<Expression> op1, op2;

public:
  Arithmetic(
    char op,
    std::unique_ptr<Expression> &&op1,
    std::unique_ptr<Expression> &&op2) :
    op(op), op1(std::move(op1)), op2(std::move(op2)) {
    assert(op == '+' || op == '-' || op == '*' || op == '/');
  }

  void dump(std::ostream &stream, unsigned indent) const override {
    stream << pad(indent) << "Arithmetic " << op << " {" << std::endl;
    op1->dump(stream, indent + 1);
    op2->dump(stream, indent + 1);
    stream << pad(indent) << "}" << suffix() << std::endl;
  }

  const Type *check(
    Logger &logger,
    const TypeDict &types,
    Symtab<const Type *> &symtab,
    const Type *curr_type) override;
};

class Object : public Expression {
  std::string name;

public:
  Object(const std::string &name) : name(name) {}

  void dump(std::ostream &stream, unsigned indent) const override {
    stream << pad(indent) << "Object " << name  << suffix() << std::endl;
  }

  const Type *check(
    Logger &logger,
    const TypeDict &types,
    Symtab<const Type *> &symtab,
    const Type *curr_type) override;
};

class Assign : public Expression {
  std::unique_ptr<Object> left;
  std::unique_ptr<Expression> expr;

public:
  Assign(std::unique_ptr<Object> &&left, std::unique_ptr<Expression> &&expr) :
    left(std::move(left)), expr(std::move(expr)) {}

  void dump(std::ostream &stream, unsigned indent) const override {
    stream << pad(indent) << "Assign {" << std::endl;
    left->dump(stream, indent + 1);
    expr->dump(stream, indent + 1);
    stream << pad(indent) << "}" << suffix() << std::endl;
  }

  const Type *check(
    Logger &logger,
    const TypeDict &types,
    Symtab<const Type *> &symtab,
    const Type *curr_type) override;
};

class Dispatch : public Expression {
  std::unique_ptr<Expression> expr;
  std::string name;
  std::vector<std::unique_ptr<Expression>> args;

public:
  Dispatch(
    std::unique_ptr<Expression> &&expr,
    std::string name,
    std::vector<std::unique_ptr<Expression>> &&args) :
    expr(std::move(expr)), name(name), args(std::move(args)) {}

  void dump(std::ostream &stream, unsigned indent) const override {
    stream << pad(indent) << "Dispatch " << name << " {" << std::endl;
    expr->dump(stream, indent + 1);
    for (const auto &arg : args) {
      arg->dump(stream, indent + 1);
    }
    stream << pad(indent) << "}" << suffix() << std::endl;
  }

  const Type *check(
    Logger &logger,
    const TypeDict &types,
    Symtab<const Type *> &symtab,
    const Type *curr_type) override;
};

class StaticDispatch : public Expression {
  std::unique_ptr<Expression> expr;
  std::string type;
  std::string name;
  std::vector<std::unique_ptr<Expression>> args;

public:
  StaticDispatch(
    std::unique_ptr<Expression> &&expr,
    std::string type,
    std::string name,
    std::vector<std::unique_ptr<Expression>> &&args) :
    expr(std::move(expr)), type(type), name(name), args(std::move(args)) {}

  void dump(std::ostream &stream, unsigned indent) const override {
    stream << pad(indent) << "StaticDispatch @" << type << "." << name << " {"
           << std::endl;
    expr->dump(stream, indent + 1);
    for (const auto &arg : args) {
      arg->dump(stream, indent + 1);
    }
    stream << pad(indent) << "}" << suffix() << std::endl;
  }

  const Type *check(
    Logger &logger,
    const TypeDict &types,
    Symtab<const Type *> &symtab,
    const Type *curr_type) override;
};

class New : public Expression {
  std::string type;

public:
  New(const std::string &type) : type(type) {}

  void dump(std::ostream &stream, unsigned indent) const override {
    stream << pad(indent) << "New @" << type << suffix() << std::endl;
  }

  const Type *check(
    Logger &logger,
    const TypeDict &types,
    Symtab<const Type *> &symtab,
    const Type *curr_type) override;
};

class Conditional : public Expression {
  std::unique_ptr<Expression> cond, then, elSe;

public:
  Conditional(
    std::unique_ptr<Expression> &&cond,
    std::unique_ptr<Expression> &&then,
    std::unique_ptr<Expression> &&elSe) :
    cond(std::move(cond)), then(std::move(then)), elSe(std::move(elSe)) {}

  void dump(std::ostream &stream, unsigned indent) const override {
    stream << pad(indent) << "Conditional {" << std::endl;
    cond->dump(stream, indent + 1);
    then->dump(stream, indent + 1);
    elSe->dump(stream, indent + 1);
    stream << pad(indent) << "}" << suffix() << std::endl;
  }

  const Type *check(
    Logger &logger,
    const TypeDict &types,
    Symtab<const Type *> &symtab,
    const Type *curr_type) override;
};

class Loop : public Expression {
  std::unique_ptr<Expression> cond, body;

public:
  Loop(std::unique_ptr<Expression> &&cond, std::unique_ptr<Expression> &&body) :
    cond(std::move(cond)), body(std::move(body)) {}

  void dump(std::ostream &stream, unsigned indent) const override {
    stream << pad(indent) << "Loop {" << std::endl;
    cond->dump(stream, indent + 1);
    body->dump(stream, indent + 1);
    stream << pad(indent) << "}" << suffix() << std::endl;
  }

  const Type *check(
    Logger &logger,
    const TypeDict &types,
    Symtab<const Type *> &symtab,
    const Type *curr_type) override;
};

/* TODO: Rename to Sequence */
class Block : public Expression {
  std::vector<std::unique_ptr<Expression>> body;

public:
  Block(std::vector<std::unique_ptr<Expression>> &&body) :
    body{std::move(body)} {
    assert(!this->body.empty());
  }

  void dump(std::ostream &stream, unsigned indent) const override {
    stream << pad(indent) << "Block {" << std::endl;
    for (const auto &expr : body) {
      expr->dump(stream, indent + 1);
    }
    stream << pad(indent) << "}" << suffix() << std::endl;
  }

  const Type *check(
    Logger &logger,
    const TypeDict &types,
    Symtab<const Type *> &symtab,
    const Type *curr_type) override;
};

class Definition : public TreeNode {
  std::string name;
  std::string type;
  std::unique_ptr<Expression> init;

public:
  Definition(
    std::string name,
    std::string type,
    std::unique_ptr<Expression> &&init = nullptr) :
    name(name), type(type), init(std::move(init)) {}

  void dump(std::ostream &stream, unsigned indent) const override {
    stream << pad(indent) << "Definition " << name << " : @" << type;
    if (init) {
      stream << " {" << std::endl;
      init->dump(stream, indent + 1);
      stream << pad(indent) << "}";
    }
    stream << std::endl;
  }

  void check(
    Logger &logger,
    const TypeDict &types,
    Symtab<const Type *> &symtab,
    const Type *curr_type) const;
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
    stream << pad(indent) << "Let {" << std::endl;
    for (const auto &def : defs) {
      def->dump(stream, indent + 1);
    }
    body->dump(stream, indent + 1);
    stream << pad(indent) << "}" << suffix() << std::endl;
  }

  const Type *check(
    Logger &logger,
    const TypeDict &types,
    Symtab<const Type *> &symtab,
    const Type *curr_type) override;
};

class CaseBranch : TreeNode {
  std::string name;
  std::string type;
  std::unique_ptr<Expression> expr;

public:
  CaseBranch(
    const std::string &name,
    const std::string &type,
    std::unique_ptr<Expression> &&expr) :
    name(name), type(type), expr(std::move(expr)) {}

  void dump(std::ostream &stream, unsigned indent) const override {
    stream << pad(indent) << "CaseBranch " << name << " : @" << type << " {"
           << std::endl;
    expr->dump(stream, indent + 1);
    stream << pad(indent) << "}" << std::endl;
  }

  std::pair<const Type *, const Type *> check(
    Logger &logger,
    const TypeDict &types,
    Symtab<const Type *> &symtab,
    const Type *curr_type) const;
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
    stream << pad(indent) << "Case {" << std::endl;
    expr->dump(stream, indent + 1);
    for (const auto &cAse : cases) {
      cAse->dump(stream, indent + 1);
    }
    stream << pad(indent) << "}" << suffix() << std::endl;
  }

  const Type *check(
    Logger &logger,
    const TypeDict &types,
    Symtab<const Type *> &symtab,
    const Type *curr_type) override;
};

#endif