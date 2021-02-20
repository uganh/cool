#ifndef COOL_LEX_H
#define COOL_LEX_H

#include <cassert>
#include <cstddef>
#include <fstream>
#include <iterator>

#include <iostream>

typedef unsigned char uchar;

class LexState final {
  unsigned lineno;
  std::ifstream stream;
  std::string yybuf;
  const char *yymar, *yycur, *yylim;

public:
  LexState(const std::string &filename) :
    lineno(1),
    stream(filename, std::ios::binary),
    yybuf(
      std::istreambuf_iterator<char>(stream), std::istreambuf_iterator<char>()),
    yycur(yybuf.c_str()),
    yylim(yycur + yybuf.length()) {
    stream.close();
  }

  LexState(const LexState &) = delete;
  LexState &operator=(const LexState &) = delete;

  ~LexState(void) = default;

  const char *yyPtr(void) const {
    return yycur;
  }

  bool eof(void) const {
    return yycur == yylim;
  }

  void yyBackup(void) {
    yymar = yycur;
  }

  void yyRestore(void) {
    yycur = yymar;
  }

  uchar yyGet(void) {
    assert(yycur <= yylim);
    return yycur < yylim ? *yycur++ : '\0';
  }

  uchar yyPeek(void) {
    assert(yycur <= yylim);
    return yycur < yylim ? yycur[0] : '\0';
  }

  void yySkip(size_t n = 1) {
    assert(yycur + n <= yylim);
    yycur += n;
  }

  void yyUnget(size_t n = 1) {
    assert(yycur - n >= yybuf.c_str());
    yycur -= n;
  }

  void newline(void) {
    ++lineno;
  }

  unsigned line(void) const {
    return lineno;
  }
};

#endif