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

#include <cctype>
#include <iostream>
#include <unordered_map>
#include <vector>

class CGenContext {
  unsigned int label;
  std::unordered_map<std::string, std::string> strConstants;
  std::unordered_map<int, std::string> intConstants;

  std::ostream &stream;

public:
  CGenContext(std::ostream &stream) : label(0), stream(stream) {}

  void cgen(const InheritanceTree &inheritanceTree, const std::vector<Program *> &programs);

  unsigned int newLabel(void) {
    return label++;
  }

  std::string getConstantLabel(const std::string &sVal) {
    auto iter = strConstants.find(sVal);
    if (iter == strConstants.cend()) {
      auto insertion = strConstants.insert({ sVal, "str_const" + std::to_string(strConstants.size()) });
      return insertion.first->second;
    } else {
      return iter->second;
    }
  }

  std::string  getConstantLabel(int iVal) {
    auto iter = intConstants.find(iVal);
    if (iter == intConstants.cend()) {
      auto insertion = intConstants.insert({ iVal, "int_const" + std::to_string(intConstants.size()) });
      return insertion.first->second;
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

  void emit_globl(const std::string &label) {
    stream << "\t.globl\t" << label << std::endl;
  }

  void emit_align(unsigned int align) {
    stream << "\t.align\t" << align << std::endl;
  }

  void emit_word(const std::string &label) {
    stream << "\t.word\t" << label << std::endl;
  }

  void emit_word(int value) {
    stream << "\t.word\t" << value << std::endl;
  }

  void emit_byte(unsigned char byte) {
    stream << "\t.byte\t" << byte << std::endl;
  }

  void emit_ascii(const std::string &value);

  /**
   * R-type instructions
   */

  void emit_sll(const std::string &rd, const std::string &rt, unsigned char shamt) {
    stream << "\tsll\t" << rd << ", " << rt << ", " << (shamt & 0x1f) << std::endl;
  }

  void emit_srl(const std::string &rd, const std::string &rt, unsigned char shamt) {
    stream << "\tsrl\t" << rd << ", " << rt << ", " << (shamt & 0x1f) << std::endl;
  }

  void emit_sra(const std::string &rd, const std::string &rt, unsigned char shamt) {
    stream << "\tsra\t" << rd << ", " << rt << ", " << (shamt & 0x1f) << std::endl;
  }

  void emit_sllv(const std::string &rd, const std::string &rt, const std::string &rs) {
    stream << "\tsllv\t" << rd << ", " << rt << ", " << rs << std::endl;
  }

  void emit_srlv(const std::string &rd, const std::string &rt, const std::string &rs) {
    stream << "\tsrlv\t" << rd << ", " << rt << ", " << rs << std::endl;
  }

  void emit_srav(const std::string &rd, const std::string &rt, const std::string &rs) {
    stream << "\tsrav\t" << rd << ", " << rt << ", " << rs << std::endl;
  }

  void emit_jr(const std::string &rs) {
    stream << "\tjr\t" << rs << std::endl;
  }

  void emit_jalr(const std::string &rd, const std::string &rs) {
    stream << "\tjalr\t" << rd << ", " << rs << std::endl;
  }

  void emit_jalr(const std::string &rs) {
    stream << "\tjalr\t" << rs << std::endl;
  }

  void emit_syscall(void) {
    stream << "\tsyscall" << std::endl;
  }

  void emit_mfhi(const std::string &rd) {
    stream << "\tmfhi\t" << rd << std::endl;
  }

  void emit_mthi(const std::string &rs) {
    stream << "\tmthi\t" << rs << std::endl;
  }

  void emit_mflo(const std::string &rd) {
    stream << "\tmflo\t" << rd << std::endl;
  }

  void emit_mtlo(const std::string &rs) {
    stream << "\tmtlo\t" << rs << std::endl;
  }

  void emit_mult(const std::string &rs, const std::string &rt) {
    stream << "\tmult\t" << rs << ", " << rt << std::endl;
  }

  void emit_multu(const std::string &rs, const std::string &rt) {
    stream << "\tmultu\t" << rs << ", " << rt << std::endl;
  }

  void emit_div(const std::string &rs, const std::string &rt) {
    stream << "\tdiv\t" << rs << ", " << rt << std::endl;
  }

  void emit_divu(const std::string &rs, const std::string &rt) {
    stream << "\tdivu\t" << rs << ", " << rt << std::endl;
  }

  void emit_add(const std::string &rd, const std::string &rs, const std::string &rt) {
    stream << "\tadd\t" << rd << ", " << rs << ", " << rt << std::endl;
  }

  void emit_addu(const std::string &rd, const std::string &rs, const std::string &rt) {
    stream << "\taddu\t" << rd << ", " << rs << ", " << rt << std::endl;
  }

  void emit_sub(const std::string &rd, const std::string &rs, const std::string &rt) {
    stream << "\tsub\t" << rd << ", " << rs << ", " << rt << std::endl;
  }

  void emit_subu(const std::string &rd, const std::string &rs, const std::string &rt) {
    stream << "\tsubu\t" << rd << ", " << rs << ", " << rt << std::endl;
  }

  void emit_and(const std::string &rd, const std::string &rs, const std::string &rt) {
    stream << "\tand\t" << rd << ", " << rs << ", " << rt << std::endl;
  }

  void emit_or(const std::string &rd, const std::string &rs, const std::string &rt) {
    stream << "\tor\t" << rd << ", " << rs << ", " << rt << std::endl;
  }

  void emit_xor(const std::string &rd, const std::string &rs, const std::string &rt) {
    stream << "\txor\t" << rd << ", " << rs << ", " << rt << std::endl;
  }

  void emit_nor(const std::string &rd, const std::string &rs, const std::string &rt) {
    stream << "\tnor\t" << rd << ", " << rs << ", " << rt << std::endl;
  }

  void emit_slt(const std::string &rd, const std::string &rs, const std::string &rt) {
    stream << "\tslt\t" << rd << ", " << rs << ", " << rt << std::endl;
  }

  void emit_sltu(const std::string &rd, const std::string &rs, const std::string &rt) {
    stream << "\tsltu\t" << rd << ", " << rs << ", " << rt << std::endl;
  }

  /**
   * J-type instructions
   */

  void emit_j(const std::string &label) {
    stream << "\tj\t" << label << std::endl;
  }

  void emit_j(unsigned int label) {
    stream << "\tj\t label" << label << std::endl;
  }

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

  void emit_blez(const std::string &rs, unsigned int label) {
    stream << "\tblez\t" << rs << ", label" << label << std::endl;
  }

  void emit_bgtz(const std::string &rs, unsigned int label) {
    stream << "\tbgtz\t" << rs << ", label" << label << std::endl;
  }

  void emit_addi(const std::string &rt, const std::string &rs, short imm) {
    stream << "\taddi\t" << rt << ", " << rs << ", " << imm << std::endl;
  }

  void emit_addiu(const std::string &rt, const std::string &rs, short imm) {
    stream << "\taddiu\t" << rt << ", " << rs << ", " << imm << std::endl;
  }

  void emit_slti(const std::string &rt, const std::string &rs, short imm) {
    stream << "\tslti\t" << rt << ", " << rs << ", " << imm << std::endl;
  }

  void emit_sltiu(const std::string &rt, const std::string &rs, short imm) {
    stream << "\tsltiu\t" << rt << ", " << rs << ", " << imm << std::endl;
  }

  void emit_andi(const std::string &rt, const std::string &rs, unsigned short imm) {
    stream << "\tandi\t" << rt << ", " << rs << ", " << imm << std::endl;
  }

  void emit_ori(const std::string &rt, const std::string &rs, unsigned short imm) {
    stream << "\tori\t" << rt << ", " << rs << ", " << imm << std::endl;
  }

  void emit_xori(const std::string &rt, const std::string &rs, unsigned short imm) {
    stream << "\txori\t" << rt << ", " << rs << ", " << imm << std::endl;
  }

  void emit_lui(const std::string &rt, unsigned short imm) {
    stream << "\tlui\t" << rt << ", " << imm << std::endl;
  }

  void emit_lb(const std::string &rt, const std::string &rs, short imm) {
    stream << "\tlb\t" << rt << ", " << imm << "(" << rs << ")" << std::endl;
  }

  void emit_lh(const std::string &rt, const std::string &rs, short imm) {
    stream << "\tlh\t" << rt << ", " << imm << "(" << rs << ")" << std::endl;
  }

  void emit_lw(const std::string &rt, const std::string &rs, short imm) {
    stream << "\tlw\t" << rt << ", " << imm << "(" << rs << ")" << std::endl;
  }

  void emit_lbu(const std::string &rt, const std::string &rs, short imm) {
    stream << "\tlbu\t" << rt << ", " << imm << "(" << rs << ")" << std::endl;
  }

  void emit_lhu(const std::string &rt, const std::string &rs, short imm) {
    stream << "\tlhu\t" << rt << ", " << imm << "(" << rs << ")" << std::endl;
  }

  void emit_sb(const std::string &rt, const std::string &rs, short imm) {
    stream << "\tsb\t" << rt << ", " << imm << "(" << rs << ")" << std::endl;
  }

  void emit_sh(const std::string &rt, const std::string &rs, short imm) {
    stream << "\tsh\t" << rt << ", " << imm << "(" << rs << ")" << std::endl;
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
    stream << "\tli\t" << dst << ", " << imm << std::endl;
  }

  void emit_lw(const std::string &dst, const std::string &label) {
    stream << "\tlw\t" << dst << ", " << label << std::endl;
  }

  void emit_la(const std::string &dst, const std::string &label) {
    stream << "\tla\t" << dst << ", " << label << std::endl;
  }

  void emit_blt(const std::string &r1, const std::string &r2, unsigned int label) {
    stream << "\tblt\t" << r1 << ", " << r2 << ", label" << label << std::endl;
  }

  void emit_ble(const std::string &r1, const std::string &r2, unsigned int label) {
    stream << "\tble\t" << r1 << ", " << r2 << ", label" << label << std::endl;
  }

  void emit_bgt(const std::string &r1, const std::string &r2, unsigned int label) {
    stream << "\tbgt\t" << r1 << ", " << r2 << ", label" << label << std::endl;
  }

  void emit_bge(const std::string &r1, const std::string &r2, unsigned int label) {
    stream << "\tbge\t" << r1 << ", " << r2 << ", label" << label << std::endl;
  }
};
