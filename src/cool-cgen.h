#pragma once

/**
 * The expected behavior of Cool programs is defned by the operational semantics
 * for Cool given in Section 13 of the *Cool Reference Manual*.
 *
 * The interface between the runtime system and the generated code is given in
 * the *Cool Runtime* manual, available on the course website.
 *
 * Additionally, you will likely need to read parts of the *Spim* manual in
 * order to understand the format of the code that you need to output.
 *
 * At a high-level, your code generator will need to perform the following
 * tasks:
 *
 *  1. Determine and emit code for global constants, such as prototype objects.
 *  2. Determine and emit code for global tables, such as the class nameTab, the
 *    class objTab, and the dispatch tables.
 *  3. Determine and emit code for the initialization method of each class.
 *  4. Determine and emit code for each method defnition.
 *
 * The runtime system contains four classes of routines:
 *
 *  1. startup code, which invokes the main method of the main program;
 *  2. the code for methods of predefined classes (Object, IO, String);
 *  3. a few special procedures needed by Cool programs to test objects for
 * equality and handle runtime errors;
 *  4. the garbage collector.
 *
 * **Objects**
 *
 * -4 | Garbage collector tag
 * ----------------------------------
 *  0 | Class tag
 * ----------------------------------
 *  4 | Object size (in 32-bit words)
 * ----------------------------------
 *  8 | Dispatch pointer
 * ----------------------------------
 *      Attributes ....
 *
 * **Register and Calling Conventions**
 *
 * Calling convention for Cool methods:
 *
 *  + The return address should be passed in $ra.
 *  + The standard callee-saved registers on the MIPS architecture $s0, $s1,
 *    $s2, $s3, $s4, $s5, $s6, and $s7, along with the frame pointer $fp are
 *    considered callee-saved by Coolaid, as well as the runtime system.
 *   + Register $s7 is used by the garbage collector and should never be touched
 *     by the generated code.
 *  + The self object must be passed in $a0, while additional arguments should
 *    be on top of the stack, first argument pushed first.
 *  + For the initializations methods, Coolaid and the runtime system consider
 *    $a0 to be callee-saved
 *
 * The following registers are used by the runtime system:
 *
 * + Scratch registers: $v0, $v1, $a0-$a2, $t0-$t4
 * + Heap pointer: $gp
 * + Limit pointer $s7
 *
 * **Runtime Interface**
 *
 */

#include "cool-tree.h"
#include "cool-type.h"

#include <iostream>
#include <unordered_map>
#include <vector>

class CGenContext {
  unsigned int label;
  std::unordered_map<std::string, unsigned int> strConstants;
  std::unordered_map<int, unsigned int> intConstants;

  std::ostream &stream;

public:
  CGenContext(std::ostream &stream) : label(0), stream(stream) {}

  void cgen(const InheritanceTree &inheritanceTree, const std::vector<Program *> &programs);

  unsigned int newLabel(void) {
    return label++;
  }

  unsigned int installConstant(const std::string &sVal) {
    auto iter = strConstants.find(sVal);
    if (iter == strConstants.cend()) {
      unsigned int index = static_cast<unsigned int>(strConstants.size());
      strConstants.insert({ sVal, index });
      return index;
    } else {
      return iter->second;
    }
  }

  unsigned int installConstant(int iVal) {
    auto iter = intConstants.find(iVal);
    if (iter == intConstants.cend()) {
      unsigned int index = static_cast<unsigned int>(strConstants.size());
      intConstants.insert({ iVal, index });
      return index;
    }
    else {
      return iter->second;
    }
  }

  void emit_label(unsigned int label_id) {
    stream << "label" << label_id << ":" << std::endl;
  }

  void emit_label(const std::string &label) {
    stream << label << ":" << std::endl;
  }

  /**
   * R-type instructions
   */

  void emit_jr(const std::string &rs) {
    stream << "\tjr\t" << rs << std::endl;
  }

  void emit_jalr(const std::string &rs) {
    stream << "\tjalr\t" << rs << std::endl;
  }

  void emit_add(const std::string &rd, const std::string &rs, const std::string &rt) {
    stream << "\tadd\t" << rd << ", " << rs << ", " << rt << std::endl;
  }

  void emit_addu(const std::string &rd, const std::string &rs, const std::string &rt) {
    stream << "\taddu\t" << rd << ", " << rs << ", " << rt << std::endl;
  }

  /**
   * J-type instructions
   */

  void emit_jal(const std::string &label) {
    stream << "\tjal\t" << label << std::endl;
  }

  /**
   * I-type instructions
   */

  void emit_beq(const std::string &rs, const std::string &rt, unsigned int label) {
    stream << "\tbeq\t" << rs << ", " << rt << ", label" << label << std::endl;
  }

  void emit_bne(const std::string &rs, const std::string &rt, unsigned int label) {
    stream << "\tbne\t" << rs << ", " << rt << ", label" << label << std::endl;
  }

  void emit_addiu(const std::string &rt, const std::string &rs, short imm) {
    stream << "\taddiu\t" << rt << ", " << rs << ", " << imm << std::endl;
  }

  void emit_lw(const std::string &rt, const std::string &rs, short imm) {
    stream << "\tlw\t" << rt << ", " << imm << "(" << rs << ")" << std::endl;
  }

  void emit_sw(const std::string &rt, const std::string &rs, short imm) {
    stream << "\tsw\t" << rt << ", " << imm << "(" << rs << ")" << std::endl;
  }

  /**
   * Pseudo-instructions
   */

  void emit_move(const std::string &dst, const std::string &src) {
    stream << "\tmove\t" << dst << ", " << src << std::endl;
  }

  void emit_li(const std::string &dst, short imm) {
    stream << "\tla\t" << dst << ", " << label << std::endl;
  }

  void emit_la(const std::string &dst, const std::string &label) {
    stream << "\tla\t" << dst << ", " << label << std::endl;
  }
};
