#include <cstdlib>
#include <iostream>
#include <memory>

#include "cool-parse.h"
#include "cool-tree.h"
#include "strtab.h"

int main(int argc, char *argv[]) {
  if (argc != 2) {
    std::cerr << "usage: " << argv[0] << " file" << std::endl;
    exit(EXIT_FAILURE);
  }

  LexState in(argv[1]);
  Strtab strtab;
  std::unique_ptr<Program> program;

  yy::parser(in, strtab, program).parse();

  program->dump(std::cout);

  return 0;
}

/*
LexState in(argv[1]);
Strtab strtab;

while (true) {
  std::cout << "#" << std::setfill(' ') << std::setw(3) << in.line() << ": ";
  switch (symbol.kind()) {
    case TOKEN(TEXT_STRING):
      std::cout << '"' << escaped_string(symbol.value.as<std::string>()) << '"';
      break;
    case TOKEN(INTEGER):
      std::cout << symbol.value.as<long>();
      break;
    case TOKEN(TYPEID):
    case TOKEN(OBJECTID):
      std::cout << *symbol.value.as<Symbol>();
      break;
    case TOKEN(ERROR):
      std::cout << "[ERROR] " << symbol.value.as<const char *>();
      break;
    case TOKEN(T_CASE):
      std::cout << "case";
      break;
    case TOKEN(T_CLASS):
      std::cout << "class";
      break;
    case TOKEN(T_ELSE):
      std::cout << "else";
      break;
    case TOKEN(T_ESAC):
      std::cout << "esac";
      break;
    case TOKEN(T_FALSE):
      std::cout << "false";
      break;
    case TOKEN(T_FI):
      std::cout << "fi";
      break;
    case TOKEN(T_IF):
      std::cout << "if";
      break;
    case TOKEN(T_IN):
      std::cout << "in";
      break;
    case TOKEN(T_INHERITS):
      std::cout << "inherits";
      break;
    case TOKEN(T_ISVOID):
      std::cout << "isvoid";
      break;
    case TOKEN(T_LET):
      std::cout << "let";
      break;
    case TOKEN(T_LOOP):
      std::cout << "loop";
      break;
    case TOKEN(T_NEW):
      std::cout << "new";
      break;
    case TOKEN(T_NOT):
      std::cout << "not";
      break;
    case TOKEN(T_OF):
      std::cout << "of";
      break;
    case TOKEN(T_POOL):
      std::cout << "pool";
      break;
    case TOKEN(T_THEN):
      std::cout << "then";
      break;
    case TOKEN(T_TRUE):
      std::cout << "true";
      break;
    case TOKEN(T_WHILE):
      std::cout << "while";
      break;
    case '(':
      std::cout << "'('";
      break;
    case ')':
      std::cout << "')'";
      break;
    case '*':
      std::cout << "'*'";
      break;
    case '+':
      std::cout << "'+'";
      break;
    case ',':
      std::cout << "','";
      break;
    case '-':
      std::cout << "'-'";
      break;
    case '.':
      std::cout << "'.'";
      break;
    case '/':
      std::cout << "'/'";
      break;
    case ':':
      std::cout << "':'";
      break;
    case ';':
      std::cout << "';'";
      break;
    case '<':
      std::cout << "'<'";
      break;
    case '=':
      std::cout << "'='";
      break;
    case '@':
      std::cout << "'@'";
      break;
    case '{':
      std::cout << "'{'";
      break;
    case '}':
      std::cout << "'}'";
      break;
    case '~':
      std::cout << "'~'";
      break;
    case TOKEN(LE):
      std::cout << "<=";
      break;
    case TOKEN(ASSIGN):
      std::cout << "<-";
      break;
    case TOKEN(DARROW):
      std::cout << "=>";
      break;
    default:
      assert(0);
  }
  std::cout << std::endl;
}
*/
