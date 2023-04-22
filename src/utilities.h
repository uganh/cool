#pragma once

#include "cool-lex.h"

#include <ostream>

void dump_token(std::ostream &out, int lineno, int token, YYSTYPE *yylval_ptr);
