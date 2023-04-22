#include "cool-lex.h"

int LexState::lex(YYSTYPE *yylval_ptr) {
  const char *yytext, *YYMARKER;

loop:
  yytext = YYCURSOR;

/*!re2c
  re2c:define:YYCTYPE = char;
  re2c:yyfill:enable = 0;

  digit = [0-9];
  alpha = [A-Za-z];
  ident = digit | alpha | "_";

  * {
    if (YYCURSOR > YYLIMIT) {
      --YYCURSOR;
      return 0;
    }

    // TODO: When an invalid character (one that can't begin any token) is
    // encountered, a string containing just that character should be returned
    // as the error string. Resume lexing at the following character.

    yylval_ptr->errs = "Invalid character";
    return TK_ERROR;
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

    yylval_ptr->errs = "EOF in comment";
    return TK_ERROR;
  }

  "*)" {
    yylval_ptr->errs = "Unmatched *)";
    return TK_ERROR;
  }

  ["] {
    // Read string.

    char c;
    std::string string_literal;

    while (YYCURSOR < YYLIMIT) {
      switch (c = *YYCURSOR++) {
        case '\0':
          yylval_ptr->errs = "String contains null character";
          return TK_ERROR;
        case '\n':
          if (*YYCURSOR == '\r') {
            ++YYCURSOR;
          }
          ++curr_lineno;
          yylval_ptr-> errs = "Unterminated string constant";
          return TK_ERROR;
        case '\r':
          if (*YYCURSOR == '\n') {
            ++YYCURSOR;
          }
          ++curr_lineno;
          yylval_ptr-> errs = "Unterminated string constant";
          return TK_ERROR;
        case '"':
          return TK_STRING;
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

    yylval_ptr->errs = "EOF in string constant";
    return TK_ERROR;
  }

  digit+ {
    // TODO
    yylval_ptr->ival = 0;
    return TK_NUMBER;
  }

  [()*+,-./:;<=@{}~] {
    return yytext[0];
  }

  "<=" {
    return TK_LE;
  }

  "<-" {
    return TK_ASSIGN;
  }

  "=>" {
    return TK_DARROW;
  }

  'case' {
    return TK_CASE;
  }

  'class' {
    return TK_CLASS;
  }

  'else' {
    return TK_ELSE;
  }

  'esac' {
    return TK_ESAC;
  }

  "f" 'alse' {
    return TK_FALSE;
  }

  'fi' {
    return TK_FI;
  }

  'if' {
    return TK_IF;
  }

  'in' {
    return TK_IN;
  }

  'inherits' {
    return TK_INHERITS;
  }

  'isvoid' {
    return TK_ISVOID;
  }

  'let' {
    return TK_LET;
  }

  'loop' {
    return TK_LOOP;
  }

  'new' {
    return TK_NEW;
  }

  'not' {
    return TK_NOT;
  }

  'of' {
    return TK_OF;
  }

  'pool' {
    return TK_POOL;
  }

  'then' {
    return TK_THEN;
  }

  "t" 'rue' {
    return TK_TRUE;
  }

  'while' {
    return TK_WHILE;
  }

  [A-Z] ident* {
    return TK_TYPEID;
  }

  [a-z] ident* {
    return TK_OBJECTID;
  }
 */
}
