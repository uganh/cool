#include "cool-tree.h"
#include "utilities.h"

static void dump_indents(std::ostream &stream, const std::vector<bool> &indents) {
  if (indents.size() > 1) {
    size_t limit = indents.size() - 1;
    for (size_t index = 0; index < limit; index++) {
      if (indents[index]) {
        stream << "  ";
      } else {
        stream << "| ";
      }
    }
  }

  if (indents.size() > 0) {
    if (indents.back()) {
      stream << "`-";
    } else {
      stream << "|-";
    }
  }
}

void Assign::dump(std::ostream &stream, std::vector<bool> &indents, const Program *program) const {
  dump_indents(stream, indents);
  stream << "Assign@" << program->getLine(this) << " " << left->to_string() << std::endl;

  indents.push_back(true);
  expr->dump(stream, indents, program);
  indents.pop_back();
}

void Dispatch::dump(std::ostream &stream, std::vector<bool> &indents, const Program *program) const {
  dump_indents(stream, indents);
  stream << "Dispatch@" << program->getLine(this) << " " << name->to_string();
  if (type != nullptr) {
    stream << "@" << type->to_string();
  }
  stream << std::endl;

  if (expr) {
    indents.push_back(args.size() == 0);
    expr->dump(stream, indents, program);
    indents.pop_back();
  }

  size_t index = 0;
  for (Expression *arg : args) {
    indents.push_back(++index == args.size());
    arg->dump(stream, indents, program);
    indents.pop_back();
  }
}

void Conditional::dump(std::ostream &stream, std::vector<bool> &indents, const Program *program) const {
  dump_indents(stream, indents);
  stream << "Conditional@" << program->getLine(this) << std::endl;

  indents.push_back(false);
  pred->dump(stream, indents, program);
  indents.pop_back();

  indents.push_back(false);
  then->dump(stream, indents, program);
  indents.pop_back();

  indents.push_back(true);
  elSe->dump(stream, indents, program);
  indents.pop_back();
}

void Loop::dump(std::ostream &stream, std::vector<bool> &indents, const Program *program) const {
  dump_indents(stream, indents);
  stream << "Loop@" << program->getLine(this) << std::endl;

  indents.push_back(false);
  pred->dump(stream, indents, program);
  indents.pop_back();

  indents.push_back(true);
  body->dump(stream, indents, program);
  indents.pop_back();
}

void Block::dump(std::ostream &stream, std::vector<bool> &indents, const Program *program) const {
  dump_indents(stream, indents);
  stream << "Block@" << program->getLine(this) << std::endl;

  size_t index = 0;
  for (Expression *expr : exprs) {
    indents.push_back(++index == exprs.size());
    expr->dump(stream, indents, program);
    indents.pop_back();
  }
}

void Definition::dump(std::ostream &stream, std::vector<bool> &indents, const Program *program) const {
  dump_indents(stream, indents);
  stream << "Definition@" << program->getLine(this) << " " << name->to_string() << " : " << type->to_string() << std::endl;

  if (init) {
    indents.push_back(true);
    init->dump(stream, indents, program);
    indents.pop_back();
  }
}

void Let::dump(std::ostream &stream, std::vector<bool> &indents, const Program *program) const {
  dump_indents(stream, indents);
  stream << "Let@" << program->getLine(this) << std::endl;

  indents.push_back(false);
  for (Definition *def : defs) {
    def->dump(stream, indents, program);
  }
  indents.pop_back();

  indents.push_back(true);
  body->dump(stream, indents, program);
  indents.pop_back();
}

void Branch::dump(std::ostream &stream, std::vector<bool> &indents, const Program *program) const {
  dump_indents(stream, indents);
  stream << "Branch@" << program->getLine(this) << " " << name->to_string() << " : " << type->to_string() << std::endl;

  indents.push_back(true);
  expr->dump(stream, indents, program);
  indents.pop_back();
}

void Case::dump(std::ostream &stream, std::vector<bool> &indents, const Program *program) const {
  dump_indents(stream, indents);
  stream << "Case@" << program->getLine(this) << std::endl;

  indents.push_back(false);
  expr->dump(stream, indents, program);
  indents.pop_back();

  size_t index = 0;
  for (Branch *branch : branches) {
    indents.push_back(++index == branches.size());
    branch->dump(stream, indents, program);
    indents.pop_back();
  }
}

void New::dump(std::ostream &stream, std::vector<bool> &indents, const Program *program) const {
  dump_indents(stream, indents);
  stream << "New@" << program->getLine(this) << " " << type->to_string() << std::endl;
}

void IsVoid::dump(std::ostream &stream, std::vector<bool> &indents, const Program *program) const {
  dump_indents(stream, indents);
  stream << "IsVoid@" << program->getLine(this) << std::endl;

  indents.push_back(true);
  expr->dump(stream, indents, program);
  indents.pop_back();
}

void Arithmetic::dump(std::ostream &stream, std::vector<bool> &indents, const Program *program) const {
  dump_indents(stream, indents);
  stream << "Arithmetic.";
  switch (op) {
    case ArithmeticOperator::ADD: stream << "ADD"; break;
    case ArithmeticOperator::SUB: stream << "SUB"; break;
    case ArithmeticOperator::MUL: stream << "MUL"; break;
    case ArithmeticOperator::DIV: stream << "DIV"; break;
  }
  stream << "@" << program->getLine(this) << std::endl;

  indents.push_back(false);
  op1->dump(stream, indents, program);
  indents.pop_back();

  indents.push_back(true);
  op2->dump(stream, indents, program);
  indents.pop_back();
}

void Complement::dump(std::ostream &stream, std::vector<bool> &indents, const Program *program) const {
  dump_indents(stream, indents);
  stream << "Complement@" << program->getLine(this) << std::endl;

  indents.push_back(true);
  expr->dump(stream, indents, program);
  indents.pop_back();
}

void Comparison::dump(std::ostream &stream, std::vector<bool> &indents, const Program *program) const {
  dump_indents(stream, indents);
  stream << "Comparison.";
  switch (op) {
    case ComparisonOperator::EQ: stream << "EQ"; break;
    case ComparisonOperator::LE: stream << "LE"; break;
    case ComparisonOperator::LT: stream << "LT"; break;
  }
  stream << "@" << program->getLine(this) << std::endl;

  indents.push_back(false);
  op1->dump(stream, indents, program);
  indents.pop_back();

  indents.push_back(true);
  op2->dump(stream, indents, program);
  indents.pop_back();
}

void Not::dump(std::ostream &stream, std::vector<bool> &indents, const Program *program) const {
  dump_indents(stream, indents);
  stream << "Not@" << program->getLine(this) << std::endl;

  indents.push_back(true);
  expr->dump(stream, indents, program);
  indents.pop_back();
}

void Object::dump(std::ostream &stream, std::vector<bool> &indents, const Program *program) const {
  dump_indents(stream, indents);
  stream << "Object@" << program->getLine(this) << " " << name->to_string() << std::endl;
}

void Integer::dump(std::ostream &stream, std::vector<bool> &indents, const Program *program) const {
  dump_indents(stream, indents);
  stream << "Integer@" << program->getLine(this) << " " << value << std::endl;
}

void String::dump(std::ostream &stream, std::vector<bool> &indents, const Program *program) const {
  dump_indents(stream, indents);
  stream << "String@" << program->getLine(this) << " \"";
  print_escaped_string(stream, value.c_str());
  stream << "\"" << std::endl;
}

void Boolean::dump(std::ostream &stream, std::vector<bool> &indents, const Program *program) const {
  dump_indents(stream, indents);
  stream << "Boolean@" << program->getLine(this) << " " << (value ? "true" : "false") << std::endl;
}

void Attribute::dump(std::ostream &stream, std::vector<bool> &indents, const Program *program) const {
  dump_indents(stream, indents);
  stream << "Attribute@" << program->getLine(this) << " " << name->to_string() << " : " << type->to_string() << std::endl;

  if (init) {
    indents.push_back(true);
    init->dump(stream, indents, program);
    indents.pop_back();
  }
}

void Formal::dump(std::ostream &stream, std::vector<bool> &indents, const Program *program) const {
  dump_indents(stream, indents);
  stream << "Formal@" << program->getLine(this) << " " << name->to_string() << " : " << type->to_string() << std::endl;
}

void Method::dump(std::ostream &stream, std::vector<bool> &indents, const Program *program) const {
  dump_indents(stream, indents);
  stream << "Method@" << program->getLine(this) << " " << name->to_string() << " : " << type->to_string() << std::endl;

  indents.push_back(false);
  for (Formal *formal : formals) {
    formal->dump(stream, indents, program);
  }
  indents.pop_back();

  indents.push_back(true);
  expr->dump(stream, indents, program);
  indents.pop_back();
}

void Class::dump(std::ostream &stream, std::vector<bool> &indents, const Program *program) const {
  dump_indents(stream, indents);
  stream << "Class@" << program->getLine(this) << " " << name->to_string();
  if (base != nullptr) {
    stream << " inherits " << base->to_string();
  }
  stream << std::endl;

  size_t index = 0;
  for (Feature *feature : features) {
    indents.push_back(++index == features.size());
    feature->dump(stream, indents, program);
    indents.pop_back();
  }
}

Program::~Program(void) noexcept {
  classes.clear();

  for (auto item : nodes) {
    delete item.first;
  }
  nodes.clear();
}

void Program::dump(std::ostream &stream) const {
  std::vector<bool> indents;

  stream << "Program \"" << name << "\"" << std::endl;

  size_t index = 0;
  for (Class *claSs : classes) {
    indents.push_back(++index == classes.size());
    claSs->dump(stream, indents, this);
    indents.pop_back();
  }
}
