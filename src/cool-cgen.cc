#include "cool-cgen.h"

#include <cassert>
#include <set>
#include <string>

class Environment {
  size_t numParams;
  Symtab<int> locals;

public:
  Environment(void) : numParams(0) {}

  explicit Environment(const std::vector<Formal *> &formals) : numParams(formals.size()) {
    for (size_t i = 0, n = formals.size(); i < n; i++) {
      locals.define(formals[i]->getName(), i * 4 + 4);
    }
  }

  void enterScope(void) {
    locals.enterScope();
  }

  void leaveScope(void) {
    locals.leaveScope();
  }

  void alloc(Symbol *name) {
    locals.define(name, -static_cast<int>(locals.size() - numParams) - 12);
  }

  int getFrameOffset(Symbol *name) const {
    int offset;
    bool ok = locals.lookup(name, offset);
    return offset;
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

  stream << "\t.text" << std::endl;

  for (Program *program : programs) {
    for (Class *claSs : program->getClasses()) {
      Symbol *typeName = claSs->getName();

      const ClassInfo *classInfo = inheritanceTree.getClassInfo(typeName);

      /* Generate initialization methods */

      unsigned int locals = 0;
      for (const AttributeInfo *attributeInfo : classInfo->attributes) {
        if (attributeInfo->locals > locals) {
          locals = attributeInfo->locals;
        }
      }

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


      // For the initialization methods, Coolaid andthe runtime system consider
      // $a0 to be callee-saved (in addition to the callee - saved registers
      // for normal methods).
      emit_move(registers::a0, registers::s0);

      emit_move(registers::sp, registers::fp);
      emit_lw(registers::ra, registers::sp, -8);
      emit_lw(registers::s0, registers::sp, -4);
      emit_lw(registers::fp, registers::sp, 0);
      emit_jr(registers::ra);
    }
  }
}

static inline std::string int_const(unsigned int index) {
  return "int_const" + std::to_string(index);
}

static inline std::string str_const(unsigned int index) {
  return "str_const" + std::to_string(index);
}

void Assign::cgen(
  CGenContext &context,
  const InheritanceTree &inheritanceTree,
  const Program *program,
  Symbol *currentType,
  Environment &env) const {
  std::cerr << __FUNCTION__ << " not implemented" << std::endl;
  // TODO:
}

void Dispatch::cgen(
  CGenContext &context,
  const InheritanceTree &inheritanceTree,
  const Program *program,
  Symbol *currentType,
  Environment &env) const {
  std::cerr << __FUNCTION__ << " not implemented" << std::endl;
  // TODO:
}

void Conditional::cgen(
  CGenContext &context,
  const InheritanceTree &inheritanceTree,
  const Program *program,
  Symbol *currentType,
  Environment &env) const {
  std::cerr << __FUNCTION__ << " not implemented" << std::endl;
  // TODO:
}

void Loop::cgen(
  CGenContext &context,
  const InheritanceTree &inheritanceTree,
  const Program *program,
  Symbol *currentType,
  Environment &env) const {
  std::cerr << __FUNCTION__ << " not implemented" << std::endl;
  // TODO:
}

void Block::cgen(
  CGenContext &context,
  const InheritanceTree &inheritanceTree,
  const Program *program,
  Symbol *currentType,
  Environment &env) const {
  std::cerr << __FUNCTION__ << " not implemented" << std::endl;
  // TODO:
}

void Let::cgen(
  CGenContext &context,
  const InheritanceTree &inheritanceTree,
  const Program *program,
  Symbol *currentType,
  Environment &env) const {
  std::cerr << __FUNCTION__ << " not implemented" << std::endl;
  // TODO:
}

void Case::cgen(
  CGenContext &context,
  const InheritanceTree &inheritanceTree,
  const Program *program,
  Symbol *currentType,
  Environment &env) const {
  std::cerr << __FUNCTION__ << " not implemented" << std::endl;
  // TODO:
}

void New::cgen(
  CGenContext &context,
  const InheritanceTree &inheritanceTree,
  const Program *program,
  Symbol *currentType,
  Environment &env) const {
  std::cerr << __FUNCTION__ << " not implemented" << std::endl;
  // TODO:
}

void IsVoid::cgen(
  CGenContext &context,
  const InheritanceTree &inheritanceTree,
  const Program *program,
  Symbol *currentType,
  Environment &env) const {
  std::cerr << __FUNCTION__ << " not implemented" << std::endl;
  // TODO:
}

void Arithmetic::cgen(
  CGenContext &context,
  const InheritanceTree &inheritanceTree,
  const Program *program,
  Symbol *currentType,
  Environment &env) const {
  std::cerr << __FUNCTION__ << " not implemented" << std::endl;
  // TODO:
}

void Complement::cgen(
  CGenContext &context,
  const InheritanceTree &inheritanceTree,
  const Program *program,
  Symbol *currentType,
  Environment &env) const {
  std::cerr << __FUNCTION__ << " not implemented" << std::endl;
  // TODO:
}

void Comparison::cgen(
  CGenContext &context,
  const InheritanceTree &inheritanceTree,
  const Program *program,
  Symbol *currentType,
  Environment &env) const {
  std::cerr << __FUNCTION__ << " not implemented" << std::endl;
  // TODO:
}

void Not::cgen(
  CGenContext &context,
  const InheritanceTree &inheritanceTree,
  const Program *program,
  Symbol *currentType,
  Environment &env) const {
  std::cerr << __FUNCTION__ << " not implemented" << std::endl;
  // TODO:
}

void Object::cgen(
  CGenContext &context,
  const InheritanceTree &inheritanceTree,
  const Program *program,
  Symbol *currentType,
  Environment &env) const {
  std::cerr << __FUNCTION__ << " not implemented" << std::endl;
  // TODO:
}

void Integer::cgen(
  CGenContext &context,
  const InheritanceTree &inheritanceTree,
  const Program *program,
  Symbol *currentType,
  Environment &env) const {
  context.emit_la(registers::a0, int_const(context.installConstant(value)));
}

void String::cgen(
  CGenContext &context,
  const InheritanceTree &inheritanceTree,
  const Program *program,
  Symbol *currentType,
  Environment &env) const {
  context.emit_la(registers::a0, str_const(context.installConstant(value)));
}

void Boolean::cgen(
  CGenContext &context,
  const InheritanceTree &inheritanceTree,
  const Program *program,
  Symbol *currentType,
  Environment &env) const {
  context.emit_la(registers::a0, value ? "bool_const1" : "bool_const0");
}