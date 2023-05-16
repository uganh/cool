#pragma once

#include "cool-lex.h"

#include <ostream>

void print_escaped_string(std::ostream& out, const char* str);

void dump_token(std::ostream &out, int line, int token, yy::parser::value_type *yylval_ptr);
