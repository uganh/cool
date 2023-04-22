
%define api.pure full

%parse-param {LexState &lexer}

%expect 0

%code requires {
class LexState;
}

%union {
long ival;
const char *errs;
}

%code {
#include "cool-lex.h"

#include <iostream>

#define yylex(yylval_ptr) lexer.lex(yylval_ptr)

void yyerror(LexState &lexer, const char *msg);
}

%token END 0

%token TK_CASE
%token TK_CLASS
%token TK_ELSE
%token TK_ESAC
%token TK_FALSE
%token TK_FI
%token TK_IF
%token TK_IN
%token TK_INHERITS
%token TK_ISVOID
%token TK_LET
%token TK_LOOP
%token TK_NEW
%token TK_NOT
%token TK_OF
%token TK_POOL
%token TK_THEN
%token TK_TRUE
%token TK_WHILE

%token TK_NUMBER
%token TK_STRING

%token TK_OBJECTID
%token TK_TYPEID

%token TK_ASSIGN "<-"
%token TK_DARROW "=>"
%token TK_LE     "<="

%token TK_ERROR

%%

Program:
    %empty
  ;

%%

void yyerror(LexState &lexer, const char *msg) {
  std::cerr << msg << std::endl;
}
