#include "cool-lex.h"
#include "utilities.h"

#include <fstream>
#include <iomanip>
#include <iostream>

int main(int argc, char *argv[]) {
  int token;
  YYSTYPE yylval;

  int opt_index = 1;

  while (opt_index < argc) {
    const char *filename = argv[opt_index++];

    std::ifstream stream(filename, std::ios::binary);
    if (!stream) {
      std::cerr << "Could not open input file " << filename << std::endl;
      return -1;
    }

    std::cout << "#name \"" << filename << "\"" << std::endl;
  
    LexState lexer(stream);
    while ((token = lexer.lex(&yylval)) != 0) {
      dump_token(std::cout, lexer.line_number(), token, &yylval);
    }

    stream.close();
  }

  return 0;
}
