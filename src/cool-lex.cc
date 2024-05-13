#include "cool-lex.h"

#include <cstdlib>

#define yyleng static_cast<size_t>(YYCURSOR - yytext)

int LexState::lex(yy::parser::value_type *yylval_ptr, yy::parser::location_type *yylloc_ptr) {
  const char *yytext;

loop:
  yytext = YYCURSOR;

  *yylloc_ptr = curr_lineno;

/*!re2c
  re2c:define:YYCTYPE = char;
  re2c:yyfill:enable = 0;

  digit = [0-9];
  alpha = [A-Za-z];
  ident = digit | alpha | "_";

  * {
    if (YYCURSOR > YYLIMIT) {
      --YYCURSOR;
      return TOKID(END);
    }

    // TODO: When an invalid character (one that can't begin any token) is
    // encountered, a string containing just that character should be returned
    // as the error string. Resume lexing at the following character.

    yylval_ptr->emplace<const char *>("Invalid character");
    return TOKID(ERROR);
  }

  [\t\v\f\r ]+ {
    goto loop;
  }

  "\n" "\r"? | "\r" "\n"? {
    ++curr_lineno;
    goto loop;
  }

  "--" {
    // Skip line comment.

    while (YYCURSOR < YYLIMIT) {
      switch (*YYCURSOR++) {
        case '\n':
          if (*YYCURSOR == '\r') {
            ++YYCURSOR;
          }
          ++curr_lineno;
          goto loop;
        case '\r':
          if (*YYCURSOR == '\n') {
            ++YYCURSOR;
          }
          ++curr_lineno;
          goto loop;
      }
    }

    return 0;
  }

  "(*" {
    // Skip comment.

    // Comment may be nested.
    int nest_level = 1;

    while (YYCURSOR < YYLIMIT) {
      switch (*YYCURSOR++) {
        case '\n':
          if (*YYCURSOR == '\r') {
            ++YYCURSOR;
          }
          ++curr_lineno;
          break;
        case '\r':
          if (*YYCURSOR == '\n') {
            ++YYCURSOR;
          }
          ++curr_lineno;
          break;
        case '(':
          if (*YYCURSOR == '*') {
            ++YYCURSOR;
            ++nest_level;
          }
          break;
        case '*':
          if (*YYCURSOR == ')') {
            ++YYCURSOR;
            if (--nest_level == 0) {
              goto loop;
            }
          }
          break;
      }
    }

    yylval_ptr->emplace<const char *>("EOF in comment");
    return TOKID(ERROR);
  }

  "*)" {
    yylval_ptr->emplace<const char *>("Unmatched *)");
    return TOKID(ERROR);
  }

  ["] {
    // Read string.

    char c;
    std::string string_literal;

    while (YYCURSOR < YYLIMIT) {
      switch (c = *YYCURSOR++) {
        case '\0':
          yylval_ptr->emplace<const char *>("String contains null character");
          return TOKID(ERROR);
        case '\n':
          if (*YYCURSOR == '\r') {
            ++YYCURSOR;
          }
          ++curr_lineno;
          yylval_ptr->emplace<const char *>("Unterminated string constant");
          return TOKID(ERROR);
        case '\r':
          if (*YYCURSOR == '\n') {
            ++YYCURSOR;
          }
          ++curr_lineno;
          yylval_ptr->emplace<const char *>("Unterminated string constant");
          return TOKID(ERROR);
        case '"':
          yylval_ptr->emplace<std::string>(string_literal);
          return TOKID(STRING);
        case '\\':
          if (YYCURSOR < YYLIMIT) {
            switch (c = *YYCURSOR++) {
              case '\n':
                if (*YYCURSOR == '\r') {
                  ++YYCURSOR;
                }
                ++curr_lineno;
                string_literal.push_back('\n');
                break;
              case '\r':
                if (*YYCURSOR == '\n') {
                  ++YYCURSOR;
                }
                ++curr_lineno;
                string_literal.push_back('\n');
                break;
              case 'a': string_literal.push_back('\a'); break;
              case 'b': string_literal.push_back('\b'); break;
              case 'f': string_literal.push_back('\f'); break;
              case 'n': string_literal.push_back('\n'); break;
              case 'r': string_literal.push_back('\r'); break;
              case 't': string_literal.push_back('\t'); break;
              case 'v': string_literal.push_back('\v'); break;
              default:
                string_literal.push_back(c);
                break;
            }
          }
          break;
        default:
          string_literal.push_back(c);
          break;
      }
    }

    yylval_ptr->emplace<const char *>("EOF in string constant");
    return TOKID(ERROR);
  }

  digit+ {
    yylval_ptr->emplace<int>(std::strtol(yytext, NULL, 10));
    return TOKID(NUMBER);
  }

  [()*+,-./:;<=@{}~] {
    return yytext[0];
  }

  "<=" {
    return TOKID(LE);
  }

  "<-" {
    return TOKID(ASSIGN);
  }

  "=>" {
    return TOKID(DARROW);
  }

  'case' {
    return TOKID(CASE);
  }

  'class' {
    return TOKID(CLASS);
  }

  'else' {
    return TOKID(ELSE);
  }

  'esac' {
    return TOKID(ESAC);
  }

  "f" 'alse' {
    return TOKID(FALSE);
  }

  'fi' {
    return TOKID(FI);
  }

  'if' {
    return TOKID(IF);
  }

  'in' {
    return TOKID(IN);
  }

  'inherits' {
    return TOKID(INHERITS);
  }

  'isvoid' {
    return TOKID(ISVOID);
  }

  'let' {
    return TOKID(LET);
  }

  'loop' {
    return TOKID(LOOP);
  }

  'new' {
    return TOKID(NEW);
  }

  'not' {
    return TOKID(NOT);
  }

  'of' {
    return TOKID(OF);
  }

  'pool' {
    return TOKID(POOL);
  }

  'then' {
    return TOKID(THEN);
  }

  "t" 'rue' {
    return TOKID(TRUE);
  }

  'while' {
    return TOKID(WHILE);
  }

  [A-Z] ident* {
    yylval_ptr->emplace<Symbol *>(strtab.new_string(std::string(yytext, yyleng)));
    return TOKID(TYPEID);
  }

  [a-z] ident* {
    yylval_ptr->emplace<Symbol *>(strtab.new_string(std::string(yytext, yyleng)));
    return TOKID(OBJECTID);
  }
 */
}
