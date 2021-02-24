#include <algorithm>
#include <cassert>
#include <cctype>
#include <cstdlib>
#include <iomanip>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>

enum Token {
  tok_number = 256,
  tok_identifier,
  tok_def,
  tok_if,
  tok_then,
  tok_else
};

static long Number;
static std::string Identifer;

static unsigned lineno = 1;

static void error(const std::string &Msg) {
  std::cerr << "syntax error at line " << lineno << ": " << Msg << std::endl;
  exit(EXIT_FAILURE);
}

static int newlabel(void) {
  static int LabelCount = 0;
  return LabelCount++;
}

static void emit(const std::string &Instr) {
  std::cout << Instr << std::endl;
}

static void emit_push(const std::string &Reg) {
  emit("\tpushq\t" + Reg);
  // emit("\taddiu\t$sp, $sp, -4");
  // emit("\tsw\t" + Reg + ", 0($sp)");
}

static void emit_pop(const std::string &Reg) {
  emit("\tpopq\t" + Reg);
  // emit("\tlw\t" + Reg + ", 0($sp)");
  // emit("\taddiu\t$sp, $sp, 4");
}

static int scan(void) {
  static int LastChar = ' ';

  while (isspace(LastChar)) {
    if (LastChar == '\n') {
      lineno++;
    }
    LastChar = std::cin.get();
  }

  if (isdigit(LastChar)) {
    std::string NumStr;
    do {
      NumStr += LastChar;
    } while (isdigit(LastChar = std::cin.get()));
    Number = std::strtol(NumStr.c_str(), NULL, 10);
    return tok_number;
  }

  if (isalpha(LastChar)) {
    Identifer = LastChar;
    while (isalnum(LastChar = std::cin.get())) {
      Identifer += LastChar;
    }

    if (Identifer == "def") {
      return tok_def;
    } else if (Identifer == "if") {
      return tok_if;
    } else if (Identifer == "then") {
      return tok_then;
    } else if (Identifer == "else") {
      return tok_else;
    } else {
      return tok_identifier;
    }
  }

  if (LastChar == EOF) {
    return EOF;
  }

  int ThisChar = LastChar;

  LastChar = std::cin.get();
  return ThisChar;
}

static int Lookahead = scan();

static std::string token_name(int token) {
  switch (token) {
    case EOF:
      return "eof";
    case tok_number:
      return "number";
    case tok_identifier:
      return "identifier";
    case tok_def:
      return "'def'";
    case tok_if:
      return "'if'";
    case tok_then:
      return "'then'";
    case tok_else:
      return "'else'";
    case '\'':
      return "'\\''";
    default:
      if (isgraph(token)) {
        return std::string("'") + static_cast<char>(token) + "'";
      } else if (token < 256) {
        std::ostringstream oss;
        oss << "'\\x" << std::setfill('0') << std::setw(2) << std::hex << token
            << "'";
        return oss.str();
      } else {
        assert(0);
      }
  }
}

static void move(void) {
  Lookahead = scan();
}

static void match(int Expect) {
  if (Expect != Lookahead) {
    error("unexpected " + token_name(Lookahead));
  }
  move();
}

void expr(const std::vector<std::string> &Env);

void term(const std::vector<std::string> &Env) {
  switch (Lookahead) {
    case tok_number:
      emit("\tmovq\t$" + std::to_string(Number) + ", %rax");
      // emit("\tli\t$a0, " + std::to_string(Number));
      move();
      break;
    case tok_identifier: {
      std::string Name = Identifer;
      move();
      unsigned Argc = 0;
      if (Lookahead == '(') {
        move();
        // Function call
        if (Lookahead != ')') {
          expr(Env);
          ++Argc;
          emit_push("%rax");
          // emit_push("$a0");
          while (Lookahead != ')') {
            match(',');
            expr(Env);
            ++Argc;
            emit_push("%rax");
            // emit_push("$a0");
          }
        }
        match(')');
        emit("\tcall\t_" + Name);
        // emit("\tjal\t" + Name);
        if (Argc) {
          emit("\taddq\t$" + std::to_string(Argc * 8) + ", %rsp");
          // emit("\tadd\t$sp, $sp, " + std::to_string(Argc * 4));
        }
      } else {
        // Variable
        auto Iter = std::find(Env.cbegin(), Env.cend(), Name);
        if (Iter == Env.cend()) {
          error("undefined variable: '" + Name + "'");
        }
        auto offset = (Env.cend() - Iter + 1) * 8;
        emit("\tmovq\t" + std::to_string(offset) + "(%rbp), %rax");
        // emit("\tlw\t$a0, " + std::to_string(offset) + "(%fp)");
      }
      break;
    }
    case tok_if: {
      move();
      expr(Env);
      emit_push("%rax");
      // emit_push("$a0");
      match('=');
      expr(Env);
      emit_pop("%r10");
      // emit_pop("$t1");
      int iffalse = newlabel();
      int after   = newlabel();
      emit("\txorq\t%r10, %rax");
      emit("\tjnz\tL" + std::to_string(iffalse));
      // emit("\tbeq\t$a0, $t1, L" + std::to_string(iftrue));
      match(tok_then);
      /* True branch */
      expr(Env);
      emit("\tjmp\tL" + std::to_string(after));
      // emit("\tj\tL" + std::to_string(after));
      match(tok_else);
      /* False branch */
      emit("L" + std::to_string(iffalse) + ":");
      expr(Env);
      emit("L" + std::to_string(after) + ":");
      break;
    }
    default:
      error("unexpected " + token_name(Lookahead));
  }
}

void expr(const std::vector<std::string> &Env) {
  term(Env);
  while (Lookahead == '+' || Lookahead == '-') {
    char Operator = Lookahead;
    move();
    emit_push("%rax");
    // emit_push("$a0");
    term(Env);
    emit_pop("%r10");
    // emit_pop("$t1");
    if (Operator == '+') {
      emit("\taddq\t%r10, %rax");
      // emit("\tadd\t$a0, $t1, $a0");
    } else {
      emit("\tsubq\t%rax, %r10");
      emit("\tmovq\t%r10, %rax");
      // emit("\tsub\t$a0, $t1, $a0");
    }
  }
}

void decl(void) {
  match(tok_def);
  std::string Name = Identifer;
  match(tok_identifier);
  match('(');
  std::vector<std::string> Env;
  if (Lookahead != ')') {
    Env.push_back(Identifer);
    match(tok_identifier);
    while (Lookahead == ',') {
      move();
      Env.push_back(Identifer);
      match(tok_identifier);
    }
  }
  match(')');
  match('=');

  emit("\t.globl\t_" + Name);
  emit("_" + Name + ":");
  /* Calling sequence */
  // emit_push("$ra");
  // emit_push("$fp");
  // emit("\tmove $fp, $sp");
  emit_push("%rbp");
  emit("\tmovq\t%rsp, %rbp");
  expr(Env);
  emit("\tmovq\t%rbp, %rsp");
  emit_pop("%rbp");
  emit("\tretq");
  // emit("\tmove $sp, $fp");
  // emit_pop("$fp");
  // emit_pop("$ra");
}

void prog(void) {
  emit("\t.text");
  decl();
  while (Lookahead == ';') {
    move();
    emit("");
    decl();
  }

  if (Lookahead != EOF) {
    error("unexpected '" + token_name(Lookahead));
  }
}

int main(void) {
  prog();
  return 0;
}