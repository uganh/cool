#include <cassert>
#include <cstdlib>
#include <iomanip>
#include <iostream>
#include <string>

#include "cool-lex.h"
#include "cool-parse.h"
#include "utilities.h"

/*!types:re2c*/

#define BEGIN(state) yystate = yyc##state
#define yyleng       (size_t)(in.yyPtr() - yytext)
#define yymore()     goto more

#define LOOP         goto loop

#define TOKEN(tokid) yy::parser::token::tokid

#define RETURN_TOKID(tokid) \
  return parser::symbol_type(tokid)

#define RETURN_TOKEN(tokid) \
  return parser::symbol_type(TOKEN(tokid))

#define RETURN_TOKEN_AND_VALUE(tokid, value) \
  return parser::symbol_type(TOKEN(tokid), value)

namespace yy {

parser::symbol_type yylex(LexState &in, Strtab &strtab) {
  const char *yytext;
  int yystate = yycINITIAL;
  unsigned nested_comments = 0;

loop:
  yytext = in.yyPtr();

more:

/*!re2c
  re2c:yyfill:enable               = 0;
  re2c:flags:input                 = custom;
  re2c:define:YYCTYPE              = uchar;
  re2c:define:YYPEEK               = "in.yyPeek";
  re2c:define:YYSKIP               = "in.yySkip";
  re2c:define:YYBACKUP             = "in.yyBackup";
  re2c:define:YYRESTORE            = "in.yyRestore";
  re2c:define:YYGETCONDITION       = yystate;
  re2c:define:YYGETCONDITION:naked = 1;

  <INITIAL> [] {
    /* Matchs nothing */
    if (in.eof()) {
      RETURN_TOKEN(END);
    }
    in.yySkip();
    RETURN_TOKEN_AND_VALUE(ERROR, "unexpected character");
  }

  <INITIAL> [\t\v\f\r ]+ {
    LOOP;
  }

  <INITIAL> "\n" {
    in.newline();
    LOOP;
  }

  <INITIAL> "--" {
    while (!in.eof()) {
      if (in.yyGet() == '\n') {
        in.newline();
        break;
      }
    }
    LOOP;
  }

  <INITIAL, COMMENT> "(*" {
    ++nested_comments;
    BEGIN(COMMENT);
    yymore();
  }

  <COMMENT> "*)" {
    if (--nested_comments == 0) {
      BEGIN(INITIAL);
      LOOP;
    }
    yymore();
  }

  <COMMENT> * {
    if (yytext[0] == '\n') {
      in.newline();
    }
    while (!in.eof()) {
      switch (in.yyPeek()) {
        case '(':
        case '*':
          yymore();
        case '\n':
          in.newline();
          // Fall through
        default:
          in.yySkip();
      }
    }
    RETURN_TOKEN_AND_VALUE(ERROR, "unexpected eof in comment");
  }

  <INITIAL> "*)" {
    RETURN_TOKEN_AND_VALUE(ERROR, "unmatched *)");
  }

  <INITIAL> ["] {
    BEGIN(STRING);
    yymore();
  }

  <STRING> [] {
    while (!in.eof()) {
      switch (in.yyGet()) {
        case '"':
          in.yyUnget();
          yymore();
        case '\n':
          in.newline();
          RETURN_TOKEN_AND_VALUE(
            ERROR, "non-escaped newline character may not appear in a string");
        case '\0':
          RETURN_TOKEN_AND_VALUE(ERROR, "string may not contain the null");
        case '\\':
          if (in.yyGet() == '\n') {
            in.newline();
          }
          // Fall through
        default:
          break;
      }
    }
    RETURN_TOKEN_AND_VALUE(ERROR, "unexpected eof in quoted string");
  }

  <STRING> ["] {
    BEGIN(INITIAL);

    if (yyleng > 1024) {
      RETURN_TOKEN_AND_VALUE(
        ERROR, "string constants may be at most 1024 characters long");
    }

    const char *ptr = yytext + 1;
    /* Note: `yyleng` depend on `yytext` */
    const char *end = yytext + yyleng - 1;
    std::string text;

    /* Processing escaped string */    
    while (ptr < end) {
      char c = *ptr++;
      if (c == '\\') {
        assert(ptr < end);
        switch (c = *ptr++) {
          case 'b':
            text += '\n';
            break;
          case 't':
            text += '\t';
            break;
          case 'n':
            text += '\n';
            break;
          case 'f':
            text += '\f';
            break;
          default:
            /* Including '"', '\\', '0' */
            text += c;
            break;
        }
      } else {
        text += c;
      }
    }

    RETURN_TOKEN_AND_VALUE(TEXT_STRING, text);
  }

  <INITIAL> [()*+,-./:;<=@{}~] {
    RETURN_TOKID(yytext[0]);
  }

  <INITIAL> "<=" {
    RETURN_TOKEN(LE);
  }

  <INITIAL> "<-" {
    RETURN_TOKEN(ASSIGN);
  }

  <INITIAL> "=>" {
    RETURN_TOKEN(DARROW);
  }

  <INITIAL> 'class' {
    RETURN_TOKEN(T_CLASS);
  }

  <INITIAL> 'else' {
    RETURN_TOKEN(T_ELSE);
  }

  <INITIAL> "f" 'alse' {
    RETURN_TOKEN(T_FALSE);
  }

  <INITIAL> 'fi' {
    RETURN_TOKEN(T_FI);
  }

  <INITIAL> 'if' {
    RETURN_TOKEN(T_IF);
  }

  <INITIAL> 'in' {
    RETURN_TOKEN(T_IN);
  }

  <INITIAL> 'inherits' {
    RETURN_TOKEN(T_INHERITS);
  }

  <INITIAL> 'isvoid' {
    RETURN_TOKEN(T_ISVOID);
  }

  <INITIAL> 'let' {
    RETURN_TOKEN(T_LET);
  }

  <INITIAL> 'loop' {
    RETURN_TOKEN(T_LOOP);
  }

  <INITIAL> 'pool' {
    RETURN_TOKEN(T_POOL);
  }

  <INITIAL> 'then' {
    RETURN_TOKEN(T_THEN);
  }

  <INITIAL> 'while' {
    RETURN_TOKEN(T_WHILE);
  }

  <INITIAL> 'case' {
    RETURN_TOKEN(T_CASE);
  }

  <INITIAL> 'esac' {
    RETURN_TOKEN(T_ESAC);
  }

  <INITIAL> 'new' {
    RETURN_TOKEN(T_NEW);
  }

  <INITIAL> 'of' {
    RETURN_TOKEN(T_OF);
  }

  <INITIAL> 'not' {
    RETURN_TOKEN(T_NOT);
  }

  <INITIAL> "t" 'rue' {
    RETURN_TOKEN(T_TRUE);
  }

  <INITIAL> [0-9]+ {
    /* Warning: Conversion errors ignored */
    RETURN_TOKEN_AND_VALUE(INTEGER, std::strtol(yytext, NULL, 10));
  }

  <INITIAL> [A-Z][a-zA-Z0-9_]* {
    RETURN_TOKEN_AND_VALUE(TYPEID, strtab.get(std::string(yytext, yyleng)));
  }

  <INITIAL> [a-z][a-zA-Z0-9_]* {
    RETURN_TOKEN_AND_VALUE(OBJECTID, strtab.get(std::string(yytext, yyleng)));
  }
 */
}
} // namespace yy
