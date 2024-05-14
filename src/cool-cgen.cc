#include "cool-cgen.h"

#include <algorithm>
#include <cassert>
#include <set>
#include <stack>
#include <string>

class Environment {
  size_t numParams;
  Symtab<int> locals;

public:
  Environment(void) : numParams(0) {}

  explicit Environment(const std::vector<Symbol *> &params) : numParams(params.size()) {
    for (size_t i = 0, n = params.size(); i < n; i++) {
      locals.define(params[i], static_cast<int>(n - i) * 4);
    }
  }

  void enterScope(void) {
    locals.enterScope();
  }

  void leaveScope(void) {
    locals.leaveScope();
  }

  int alloc(Symbol *name) {
    int offset = -static_cast<int>(locals.size() - numParams) - 12;
    locals.define(name, offset);
    return offset;
  }

  bool getFrameOffset(Symbol *name, int &offset) const {
    return locals.lookup(name, offset);
  }
};

class EnvironmentGuard {
  Environment &env;

public:
  explicit EnvironmentGuard(Environment &env) : env(env) {
    env.enterScope();
  }

  ~EnvironmentGuard(void) noexcept {
    env.leaveScope();
  }
};

#define REGISTER(name) static const char *name = "$" #name

namespace registers {
  // constant 0
  REGISTER(zero);
  // assembler temporary
  REGISTER(at);
  // values for function returns and expression evaluation
  REGISTER(v0);
  REGISTER(v1);
  // function arguments
  REGISTER(a0);
  REGISTER(a1);
  REGISTER(a2);
  REGISTER(a3);
  // temporaries
  REGISTER(t0);
  REGISTER(t1);
  REGISTER(t2);
  REGISTER(t3);
  REGISTER(t4);
  REGISTER(t5);
  REGISTER(t6);
  REGISTER(t7);
  // saved temporaries
  REGISTER(s0);
  REGISTER(s1);
  REGISTER(s2);
  REGISTER(s3);
  REGISTER(s4);
  REGISTER(s5);
  REGISTER(s6);
  REGISTER(s7);
  // temporaries
  REGISTER(t8);
  REGISTER(t9);
  // reserved for OS kernel
  REGISTER(k0);
  REGISTER(k1);
  // global pointer
  REGISTER(gp);
  // stack pointer
  REGISTER(sp);
  // frame pointer
  REGISTER(fp);
  // return address
  REGISTER(ra);
}

void CGenContext::cgen(const InheritanceTree &inheritanceTree, const std::vector<Program *> &programs) {
  // Perform code generation in two passes: The first pass decides the object
  // layout for each class, particularly the offset at which each attribute is
  // stored in an object. Using this information, the second pass recursively
  // walks each feature and generates stack machine code for each expression.

  const ClassInfo *Int_classInfo = inheritanceTree.getClassInfo(Symbol::Int);
  const ClassInfo *String_classInfo = inheritanceTree.getClassInfo(Symbol::String);
  const ClassInfo *Bool_classInfo = inheritanceTree.getClassInfo(Symbol::Bool);

  std::vector<const ClassInfo *> classes = {
    inheritanceTree.getClassInfo(Symbol::Object),
    inheritanceTree.getClassInfo(Symbol::IO),
    Int_classInfo,
    String_classInfo,
    Bool_classInfo,
  };

  emit_globl("Main_protObj");
  emit_globl("Main_init");

  emit_globl("Main.main");
  
  emit_globl("Int_protObj");
  emit_globl("Int_init");

  emit_globl("String_protObj");
  emit_globl("String_init");

  emit_globl("_int_tag");
  emit_globl("_bool_tag");
  emit_globl("_string_tag");

  emit_globl("bool_const0");

  emit_globl("class_nameTab");

  emit_globl("class_objTab");

  stream << "\t.text" << std::endl;

  // TODO: Primitives

  for (Program *program : programs) {
    for (Class *claSs : program->getClasses()) {
      Symbol *typeName = claSs->getName();

      const ClassInfo *classInfo = inheritanceTree.getClassInfo(typeName);

      classes.push_back(classInfo);

      /* Initialization methods */

      unsigned int locals = 0;
      for (const auto &item : classInfo->attributes) {
        const AttributeInfo *attributeInfo = item.second;
        if (attributeInfo->locals > locals) {
          locals = attributeInfo->locals;
        }
      }

      Environment env;

      emit_label(classInfo->typeName->to_string() + "_init");

      emit_sw(registers::fp, registers::sp, 0);
      emit_sw(registers::s0, registers::sp, -4);
      emit_sw(registers::ra, registers::sp, -8);
      emit_move(registers::fp, registers::sp);
      emit_addiu(registers::sp, registers::sp, -12 - static_cast<int>(locals) * 4);

      emit_move(registers::s0, registers::a0);

      if (classInfo->base) {
        emit_jal(classInfo->base->typeName->to_string() + "_init");
      }

      for (const auto &item : classInfo->attributes) {
        const AttributeInfo *attributeInfo = item.second;
        if (attributeInfo->init) {
          attributeInfo->init->cgen(*this, inheritanceTree, program, typeName, env);
          emit_sw(registers::a0, registers::s0, 12 + attributeInfo->wordOffset * 4);
        }
      }

      // For the initialization methods, Coolaid andthe runtime system consider
      // $a0 to be callee-saved (in addition to the callee - saved registers
      // for normal methods).
      emit_move(registers::a0, registers::s0);

      emit_move(registers::sp, registers::fp);
      emit_lw(registers::ra, registers::sp, -8);
      emit_lw(registers::s0, registers::sp, -4);
      emit_lw(registers::fp, registers::sp, 0);
      emit_jr(registers::ra);

      /* Methods */

      for (const auto &item : classInfo->methods) {
        Symbol *methodName = item.first;
        const MethodInfo *methodInfo = item.second;

        std::vector<Symbol *> params;
        for (auto &paramDecl : methodInfo->methType.paramDecls) {
          params.push_back(paramDecl.first);
        }

        Environment env(params);

        emit_label(classInfo->typeName->to_string() + "." + methodName->to_string());

        emit_sw(registers::fp, registers::sp, 0);
        emit_sw(registers::s0, registers::sp, -4);
        emit_sw(registers::ra, registers::sp, -8);
        emit_move(registers::fp, registers::sp);
        emit_addiu(registers::sp, registers::sp, -12 - static_cast<int>(methodInfo->locals) * 4);

        emit_move(registers::s0, registers::a0);

        methodInfo->expr->cgen(*this, inheritanceTree, program, typeName, env);

        emit_move(registers::sp, registers::fp);
        emit_lw(registers::ra, registers::sp, -8);
        emit_lw(registers::s0, registers::sp, -4);
        emit_lw(registers::fp, registers::sp, 0);
        emit_addiu(registers::sp, registers::sp, static_cast<int>(params.size()) * 4);
        emit_jr(registers::ra);
      }
    }
  }

  std::sort(classes.begin(), classes.end(), [] (const ClassInfo *lhs, const ClassInfo *rhs) {
    return lhs->tag < rhs->tag;
  });

  stream << "\t.data" << std::endl;
  stream << "\t.align\t2" << std::endl;

  /* Constants */

  emit_label("_int_tag");
  emit_word(Int_classInfo->tag);

  emit_label("_bool_tag");
  emit_word(Bool_classInfo->tag);

  emit_label("_string_tag");
  emit_word(String_classInfo->tag);

  // Constants: class_nameTab
  emit_label("class_nameTab");
  for (const ClassInfo *classInfo : classes) {
    emit_word(getConstantLabel(classInfo->typeName->to_string()));
  }

  // Constants: class_objTab
  emit_label("class_objTab");
  for (const ClassInfo *classInfo : classes) {
    emit_word(classInfo->typeName->to_string() + "_protObj");
    emit_word(classInfo->typeName->to_string() + "_init");
  }

  for (const ClassInfo *classInfo : classes) {
    emit_label(classInfo->typeName->to_string() + "_dispTab");
    for (const MethodInfo *methodInfo : classInfo->dispatchTable) {
      emit_word(methodInfo->typeName->to_string() + "." + methodInfo->methName->to_string());
    }
  }

  for (const ClassInfo *classInfo : classes) {
    emit_word(-1);
    emit_label(classInfo->typeName->to_string() + "_protObj");
    emit_word(classInfo->tag);
    emit_word(3 + classInfo->wordSize);
    emit_word(classInfo->typeName->to_string() + "_dispTab");
    if (classInfo->isPrimitive) {
      if (classInfo->typeName == Symbol::String) {
        emit_word(getConstantLabel(0));
        emit_word(0);
      }
      else if (classInfo->typeName == Symbol::Int || classInfo->typeName == Symbol::Bool) {
        emit_word(0);
      }
    }
    else {
      std::stack<const ClassInfo *> ancestors;
      for (const ClassInfo *iter = classInfo; iter; iter = iter->base) {
        ancestors.push(iter);
      }

      while (!ancestors.empty()) {
        const ClassInfo *classInfo = ancestors.top();
        ancestors.pop();

        for (auto &item : classInfo->attributes) {
          const AttributeInfo *attributeInfo = item.second;
          if (attributeInfo->attrType == Symbol::Int) {
            emit_word(getConstantLabel(0));
          }
          else if (attributeInfo->attrType == Symbol::String) {
            emit_word(getConstantLabel(""));
          }
          else if (attributeInfo->attrType == Symbol::Bool) {
            emit_word("bool_const0");
          }
          else {
            emit_word(0);
          }
        }
      }
    }
  }

  for (auto &item : strConstants) {
    unsigned int size = static_cast<unsigned int>(item.first.size());
    emit_word(-1);
    emit_label(item.second);
    emit_word(String_classInfo->tag);
    emit_word(3 + 1 + (size + 1) / 4);
    emit_word(Symbol::String->to_string() + "_dispTab");
    emit_word(getConstantLabel(size));
    emit_ascii(item.first);
    emit_byte(0);
    emit_align(2);
  }

  for (auto &item : intConstants) {
    emit_word(-1);
    emit_label(item.second);
    emit_word(Int_classInfo->tag);
    emit_word(3 + Int_classInfo->wordSize);
    emit_word(Symbol::Int->to_string() + "_dispTab");
    emit_word(item.first);
  }

  emit_word(-1);
  emit_label("bool_const0");
  emit_word(Bool_classInfo->tag);
  emit_word(3 + Bool_classInfo->wordSize);
  emit_word(Symbol::Bool->to_string() + "_dispTab");
  emit_word(0);

  emit_word(-1);
  emit_label("bool_const1");
  emit_word(Bool_classInfo->tag);
  emit_word(3 + Bool_classInfo->wordSize);
  emit_word(Symbol::Bool->to_string() + "_dispTab");
  emit_word(1);
}

void CGenContext::emit_ascii(const std::string &value) {
  static const char *digits = "0123456789abcdef";
  stream << "\t.ascii\t\"";
  for (unsigned char c : value) {
    switch (c) {
    case '\0':
      stream << "\\0";
      break;
    case '\a':
      stream << "\\a";
      break;
    case '\b':
      stream << "\\b";
      break;
    case '\t':
      stream << "\\t";
      break;
    case '\v':
      stream << "\\v";
      break;
    case '\f':
      stream << "\\f";
      break;
    case '\n':
      stream << "\\n";
      break;
    case '\r':
      stream << "\\r";
      break;
    case '"':
      stream << "\\\"";
      break;
    case '\\':
      stream << "\\\\";
      break;
    default:
      if (std::isprint(c)) {
        stream << static_cast<char>(c);
      }
      else {
        stream << "\\x" << digits[c / 4] << digits[c % 16];
      }
      break;
    }
  }
  stream << "\"" << std::endl;
}

void Assign::cgen(
  CGenContext &context,
  const InheritanceTree &inheritanceTree,
  const Program *program,
  Symbol *currentType,
  Environment &env) const {
  expr->cgen(context, inheritanceTree, program, currentType, env);

  int frameOffset;
  if (env.getFrameOffset(left, frameOffset)) {
    context.emit_sw(registers::a0, registers::fp, frameOffset);
  }
  else if (const AttributeInfo *attributeInfo = inheritanceTree.getAttributeInfo(currentType, left)) {
    context.emit_sw(registers::a0, registers::s0, 12 + attributeInfo->wordOffset * 4);
  }
  else {
    assert(false);
  }
}

void Dispatch::cgen(
  CGenContext &context,
  const InheritanceTree &inheritanceTree,
  const Program *program,
  Symbol *currentType,
  Environment &env) const {
  unsigned int label = context.newLabel();

  for (Expression *arg : args) {
    arg->cgen(context, inheritanceTree, program, currentType, env);
    context.emit_sw(registers::a0, registers::sp, 0);
    context.emit_addiu(registers::sp, registers::sp, -4);
  }

  Symbol *dispatchType = nullptr;
  if (expr) {
    expr->cgen(context, inheritanceTree, program, currentType, env);
    dispatchType = expr->getStaticType();
  }
  else {
    context.emit_move(registers::a0, registers::s0);
    dispatchType = currentType;
  }

  // Runtime error check: dispatch on void
  context.emit_bne(registers::a0, registers::zero, label);
  context.emit_la(registers::a0, context.getConstantLabel(program->getName()));
  context.emit_li(registers::t1, program->getLine(this));
  context.emit_jal("_dispatch_abort");

  context.emit_label(label);
  if (type) {
    context.emit_jal(type->to_string() + "." + name->to_string());
  }
  else {
    const MethodInfo *methodInfo = inheritanceTree.getMethodInfo(currentType, name);
    context.emit_lw(registers::t1, registers::a0, 8);
    context.emit_lw(registers::t1, registers::t1, methodInfo->index * 4);
    context.emit_jalr(registers::t1);
  }
}

void Conditional::cgen(
  CGenContext &context,
  const InheritanceTree &inheritanceTree,
  const Program *program,
  Symbol *currentType,
  Environment &env) const {
  unsigned int false_branch = context.newLabel();
  unsigned int end_if = context.newLabel();

  pred->cgen(context, inheritanceTree, program, currentType, env);
  context.emit_lw(registers::t1, registers::a0, 12);
  context.emit_beq(registers::t1, registers::zero, false_branch);
  then->cgen(context, inheritanceTree, program, currentType, env);
  context.emit_j(end_if);
  context.emit_label(false_branch);
  elSe->cgen(context, inheritanceTree, program, currentType, env);
  context.emit_label(end_if);
}

void Loop::cgen(
  CGenContext &context,
  const InheritanceTree &inheritanceTree,
  const Program *program,
  Symbol *currentType,
  Environment &env) const {
  unsigned int repeat = context.newLabel();
  unsigned int end_loop = context.newLabel();

  context.emit_label(repeat);
  pred->cgen(context, inheritanceTree, program, currentType, env);
  context.emit_lw(registers::t1, registers::a0, 12);
  context.emit_beq(registers::t1, registers::zero, end_loop);
  body->cgen(context, inheritanceTree, program, currentType, env);
  context.emit_j(repeat);
  context.emit_label(end_loop);
  context.emit_move(registers::a0, registers::zero);
}

void Block::cgen(
  CGenContext &context,
  const InheritanceTree &inheritanceTree,
  const Program *program,
  Symbol *currentType,
  Environment &env) const {
  for (Expression *expr : exprs) {
    expr->cgen(context, inheritanceTree, program, currentType, env);
  }
}

void Definition::cgen(
  CGenContext &context,
  const InheritanceTree &inheritanceTree,
  const Program *program,
  Symbol *currentType,
  Environment &env) const {
  if (init) {
    init->cgen(context, inheritanceTree, program, currentType, env);
    int frameOffset = env.alloc(name);
    context.emit_sw(registers::a0, registers::fp, frameOffset);
  }
  else {
    env.alloc(name);
  }
}

void Let::cgen(
  CGenContext &context,
  const InheritanceTree &inheritanceTree,
  const Program *program,
  Symbol *currentType,
  Environment &env) const {
  EnvironmentGuard eg(env);

  for (Definition *def : defs) {
    def->cgen(context, inheritanceTree, program, currentType, env);
  }
  body->cgen(context, inheritanceTree, program, currentType, env);
}

void Branch::cgen(
  CGenContext &context,
  const InheritanceTree &inheritanceTree,
  const Program *program,
  Symbol *currentType,
  Environment &env,
  unsigned int esac_label) const {
  EnvironmentGuard eg(env);

  int next_label = context.newLabel();
  const ClassInfo *classInfo = inheritanceTree.getClassInfo(type);

  int frameOffset = env.alloc(name);
  context.emit_sw(registers::a0, registers::fp, frameOffset);

  context.emit_li(registers::t2, classInfo->tag);
  context.emit_blt(registers::t1, registers::t2, next_label);
  context.emit_li(registers::t2, classInfo->tagEnd);
  context.emit_bge(registers::t1, registers::t2, next_label);
  expr->cgen(context, inheritanceTree, program, currentType, env);
  context.emit_j(esac_label);

  context.emit_label(next_label);
}

void Case::cgen(
  CGenContext &context,
  const InheritanceTree &inheritanceTree,
  const Program *program,
  Symbol *currentType,
  Environment &env) const {
  unsigned int case_label = context.newLabel();
  unsigned int esac_label = context.newLabel();

  expr->cgen(context, inheritanceTree, program, currentType, env);

  // Runtime error check: case on void
  context.emit_bne(registers::a0, registers::zero, case_label);
  context.emit_la(registers::a0, context.getConstantLabel(program->getName()));
  context.emit_li(registers::t1, program->getLine(this));
  context.emit_jal("_case_abort2");

  context.emit_label(case_label);
  context.emit_lw(registers::t1, registers::a0, 0); // Class tag

  std::vector<std::pair<unsigned int, Branch*>> sortedBranches;
  for (Branch *branch : branches) {
    sortedBranches.push_back({ inheritanceTree.getClassInfo(branch->getType())->tag, branch });
  }
  std::sort(sortedBranches.begin(), sortedBranches.end());
  std::reverse(sortedBranches.begin(), sortedBranches.end());

  for (auto &item : sortedBranches) {
    item.second->cgen(context, inheritanceTree, program, currentType, env, esac_label);
  }

  // Runtime error check: missing branch
  context.emit_jal("_case_abort");

  context.emit_label(esac_label);
}

void New::cgen(
  CGenContext &context,
  const InheritanceTree &inheritanceTree,
  const Program *program,
  Symbol *currentType,
  Environment &env) const {
  if (type == Symbol::SELF_TYPE) {
    context.emit_la(registers::t1, "class_ObjTab");
    context.emit_lw(registers::t2, registers::s0, 0);
    context.emit_sll(registers::t2, registers::t2, 3);
    context.emit_addu(registers::t1, registers::t1, registers::t2);
    context.emit_lw(registers::a0, registers::t1, 0); // <Class>_protObj
    context.emit_sw(registers::t1, registers::sp, 0);
    context.emit_addiu(registers::sp, registers::sp, -4);
    context.emit_jal("Object.copy");
    context.emit_addiu(registers::sp, registers::sp, 4);
    context.emit_lw(registers::t1, registers::sp, 0);
    context.emit_lw(registers::t1, registers::t1, 4); // <Class>_init
    context.emit_jalr(registers::t1);
  }
  else {
    context.emit_la(registers::a0, type->to_string() + "_protObj");
    context.emit_jal("Object.copy");
    context.emit_jal(type->to_string() + "_init");
  }
}

void IsVoid::cgen(
  CGenContext &context,
  const InheritanceTree &inheritanceTree,
  const Program *program,
  Symbol *currentType,
  Environment &env) const {
  unsigned int label = context.newLabel();

  context.emit_move(registers::t1, registers::a0);
  context.emit_la(registers::a0, "bool_const0");
  context.emit_bne(registers::t1, registers::zero, label);
  context.emit_la(registers::a0, "bool_const1");
  context.emit_label(label);
}

void Arithmetic::cgen(
  CGenContext &context,
  const InheritanceTree &inheritanceTree,
  const Program *program,
  Symbol *currentType,
  Environment &env) const {
  op1->cgen(context, inheritanceTree, program, currentType, env);
  context.emit_sw(registers::a0, registers::sp, 0);
  context.emit_addiu(registers::sp, registers::sp, -4);
  op2->cgen(context, inheritanceTree, program, currentType, env);
  context.emit_jal("Object.copy");
  context.emit_addiu(registers::sp, registers::sp, 4);
  context.emit_lw(registers::t1, registers::sp, 0);
  context.emit_lw(registers::t1, registers::t1, 12); // op1
  context.emit_lw(registers::t2, registers::a0, 12); // op2
  switch (op) {
    case ArithmeticOperator::ADD:
      context.emit_add(registers::t1, registers::t1, registers::t2);
      break;
    case ArithmeticOperator::SUB:
      context.emit_sub(registers::t1, registers::t1, registers::t2);
      break;
    case ArithmeticOperator::MUL:
      context.emit_mult(registers::t1, registers::t2);
      context.emit_mflo(registers::t1);
      break;
    case ArithmeticOperator::DIV:
      context.emit_div(registers::t1, registers::t2);
      context.emit_mflo(registers::t1);
      break;
    default:
      assert(false);
  }
  context.emit_sw(registers::t1, registers::a0, 12);
}

void Complement::cgen(
  CGenContext &context,
  const InheritanceTree &inheritanceTree,
  const Program *program,
  Symbol *currentType,
  Environment &env) const {
  expr->cgen(context, inheritanceTree, program, currentType, env);
  context.emit_jal("Object.copy");
  context.emit_lw(registers::t1, registers::a0, 12);
  context.emit_sub(registers::t1, registers::zero, registers::t1);
  context.emit_sw(registers::t1, registers::a0, 12);
}

void Comparison::cgen(
  CGenContext &context,
  const InheritanceTree &inheritanceTree,
  const Program *program,
  Symbol *currentType,
  Environment &env) const {
  unsigned int label = context.newLabel();

  op1->cgen(context, inheritanceTree, program, currentType, env);
  context.emit_sw(registers::a0, registers::sp, 0);
  context.emit_addiu(registers::sp, registers::sp, -4);
  op2->cgen(context, inheritanceTree, program, currentType, env);
  context.emit_addiu(registers::sp, registers::sp, 4);
  context.emit_lw(registers::t1, registers::sp, 0);
  context.emit_lw(registers::t1, registers::t1, 12); // op1
  context.emit_lw(registers::t2, registers::a0, 12); // op2
  context.emit_la(registers::a0, "bool_const1");
  switch (op) {
    case ComparisonOperator::LT:
      context.emit_blt(registers::t1, registers::t2, label);
      break;
    case ComparisonOperator::LE:
      context.emit_ble(registers::t1, registers::t2, label);
      break;
    case ComparisonOperator::EQ:
      context.emit_beq(registers::t1, registers::t2, label);
      break;
    default:
      assert(false);
  }
  context.emit_la(registers::a0, "bool_const0");
  context.emit_label(label);
}

void Not::cgen(
  CGenContext &context,
  const InheritanceTree &inheritanceTree,
  const Program *program,
  Symbol *currentType,
  Environment &env) const {
  context.emit_jal("Object.copy");
  context.emit_lw(registers::t1, registers::a0, 12);
  context.emit_nor(registers::t1, registers::t1, registers::t1);
  context.emit_sw(registers::t1, registers::a0, 12);
}

void Object::cgen(
  CGenContext &context,
  const InheritanceTree &inheritanceTree,
  const Program *program,
  Symbol *currentType,
  Environment &env) const {
  int frameOffset;
  if (name == Symbol::self) {
    context.emit_move(registers::a0, registers::s0);
  }
  else if (env.getFrameOffset(name, frameOffset)) {
    context.emit_lw(registers::a0, registers::fp, frameOffset);
  }
  else if (const AttributeInfo *attributeInfo = inheritanceTree.getAttributeInfo(currentType, name)) {
    context.emit_lw(registers::a0, registers::s0, 12 + attributeInfo->wordOffset * 4);
  }
  else {
    assert(false);
  }
}

void Integer::cgen(
  CGenContext &context,
  const InheritanceTree &inheritanceTree,
  const Program *program,
  Symbol *currentType,
  Environment &env) const {
  context.emit_la(registers::a0, context.getConstantLabel(value));
}

void String::cgen(
  CGenContext &context,
  const InheritanceTree &inheritanceTree,
  const Program *program,
  Symbol *currentType,
  Environment &env) const {
  context.emit_la(registers::a0, context.getConstantLabel(value));
}

void Boolean::cgen(
  CGenContext &context,
  const InheritanceTree &inheritanceTree,
  const Program *program,
  Symbol *currentType,
  Environment &env) const {
  context.emit_la(registers::a0, value ? "bool_const1" : "bool_const0");
}