#include "cool-lex.h"
#include "cool-semant.h"
#include "utilities.h"

#include <fstream>
#include <iomanip>
#include <iostream>
#include <vector>

int main(int argc, char *argv[]) {
  int opt_index = 1;

  std::vector<Program *> programs;

  while (opt_index < argc) {
    const char *filename = argv[opt_index++];

    std::ifstream stream(filename, std::ios::binary);
    if (!stream) {
      std::cerr << "Could not open input file " << filename << std::endl;
      return -1;
    }

    LexState lexer(stream);
    stream.close();

#if 0
    std::cout << "#name \"" << filename << "\"" << std::endl;

    int token;
    yy::parser::value_type yylval;
    yy::parser::location_type yylloc;

    while ((token = lexer.lex(&yylval, &yylloc)) != 0) {
      dump_token(std::cout, yylloc, token, &yylval);
    }
#else
    Program *program = new Program(filename);
    programs.push_back(program);

    if (yy::parser(lexer, program).parse() != 0) {
      std::cerr << "Compilation halted due to lex or parse errors" << std::endl;
    } else {
      program->dump(std::cout);
    }
#endif
  }

  InheritanceTree inheritanceTree;

  if (!semant(inheritanceTree, programs)) {
    std::cerr << "Compilation halted due to static semantic errors." << std::endl;
  }

  for (Program *program : programs) {
    delete program;
  }

  return 0;
}
