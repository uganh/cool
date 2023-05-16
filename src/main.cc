#include "cool-lex.h"
#include "utilities.h"

#include <fstream>
#include <iomanip>
#include <iostream>

int main(int argc, char *argv[]) {
  int opt_index = 1;

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

    while ((token = lexer.lex(&yylval)) != 0) {
      dump_token(std::cout, lexer.line_number(), token, &yylval);
    }
#else
    ASTContext context;

    if (yy::parser(lexer, context).parse() != 0) {
      std::cerr << "Compilation halted due to lex or parse errors" << std::endl;
      return -1;
    }
#endif
  }

  return 0;
}
