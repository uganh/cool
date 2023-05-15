#pragma once

#include "cool-parse.gen.h"

#include <istream>
#include <iterator>
#include <string>

class LexState {
  std::string yybuf;
  const char *YYCURSOR, *YYLIMIT;
  unsigned int curr_lineno;

public:
  explicit LexState(std::istream &stream)
    : yybuf(std::istreambuf_iterator<char>(stream), std::istreambuf_iterator<char>())
    , YYCURSOR(yybuf.c_str())
    , YYLIMIT(YYCURSOR + yybuf.length())
    , curr_lineno(1) {}
  
  LexState(const LexState &) = delete;
  LexState &operator=(const LexState &) = delete;

  int lex(yy::parser::value_type *yylval_ptr);

  unsigned int line_number(void) const {
    return curr_lineno;
  }
};
