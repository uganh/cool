%require "3.2"
%language "C++"

%expect 0

%locations

%define api.location.type {unsigned int}

%define api.value.type variant

%define parse.error custom

%parse-param {LexState &lexer} {Program *program}

%code requires {
#include "cool-tree.h"
#include "strtab.h"

#if defined(_MSC_VER)
# pragma warning(disable: 4065)
#endif

class LexState;
}

%code provides {
#define TOKID(name) yy::parser::token::name
}

%code {
#include "cool-lex.h"
#include "utilities.h"

#include <iostream>

#define YYLLOC_DEFAULT(Cur, Rhs, N)                                            \
  do {                                                                         \
    if (N) {                                                                   \
      (Cur) = YYRHSLOC(Rhs, 1);                                                \
    } else {                                                                   \
      (Cur) = YYRHSLOC(Rhs, 0);                                                \
    }                                                                          \
  } while (0)

#define yylex(yylval_ptr, yylloc_ptr) lexer.lex(yylval_ptr, yylloc_ptr)
}

%token END 0 "end of file"

%token CASE
%token CLASS
%token ELSE
%token ESAC
%token FALSE
%token FI
%token IF
%token IN
%token INHERITS
%token ISVOID
%token LET
%token LOOP
%token NEW
%token NOT
%token OF
%token POOL
%token THEN
%token TRUE
%token WHILE

%token <int> NUMBER
%token <std::string> STRING

%token <Symbol *> OBJECTID
%token <Symbol *> TYPEID

%token ASSIGN "<-"
%token DARROW "=>"
%token LE     "<="

%token <const char *> ERROR

/**
 * The lowest priority is used for trivial shift-reduce conflicts resolution.
 */
%precedence DEFAULT

%right "<-"
%precedence NOT
%nonassoc "<=" '<' '='
%left '+' '-'
%left '*' '/'
%precedence ISVOID
%precedence '~'
%precedence '@'
%precedence '.'

%nterm <Class *> Class

%nterm <Feature *> Feature
%nterm <std::vector<Feature *>> Features

%nterm <Formal *> Formal
%nterm <std::vector<Formal *>> Formals OptionalFormals

%nterm <Definition *> Definition
%nterm <std::vector<Definition *>> Definitions

%nterm <Branch *> Branch
%nterm <std::vector<Branch *>> Branches

%nterm <Expression *> Expression
%nterm <std::vector<Expression *>> Expressions OptionalExpressions

%nterm <std::vector<Expression *>> ExpressionList

%%

Program:
    Classes
  ;

Classes:
    Class {
      program->addClass($1);
    }
  | Classes Class {
      program->addClass($2);
    }
  ;

Class:
    CLASS TYPEID '{' Features '}' ';' {
      $$ = program->new_tree_node<Class>(@$, $2, Symbol::Object, std::move($4));
    }
  | CLASS TYPEID INHERITS TYPEID '{' Features '}' ';' {
      $$ = program->new_tree_node<Class>(@$, $2, $4, std::move($6));
    }
  ;

Features:
    %empty {
      $$ = {};
    }
  | Features Feature {
      $$ = std::move($1);
      $$.push_back($2);
    }
  ;

Feature:
    OBJECTID '(' OptionalFormals ')' ':' TYPEID '{' Expression '}' ';' {
      $$ = program->new_tree_node<Method>(@$, $1, std::move($3), $6, $8);
    }
  | OBJECTID ':' TYPEID ';' {
      $$ = program->new_tree_node<Attribute>(@$, $1, $3);
    }
  | OBJECTID ':' TYPEID "<-" Expression ';' {
      $$ = program->new_tree_node<Attribute>(@$, $1, $3, $5);
    }
  ;

OptionalFormals:
    %empty {
      $$ = {};
    }
  | Formals {
      $$ = std::move($1);
    }
  ;

Formals:
    Formal {
      $$ = std::vector<Formal *>();
      $$.push_back($1);
    }
  | Formals ',' Formal {
      $$ = std::move($1);
      $$.push_back($3);
    }
  ;

Formal:
    OBJECTID ':' TYPEID {
      $$ = program->new_tree_node<Formal>(@$, $1, $3);
    }
  ;

Expression:
    OBJECTID "<-" Expression {
      $$ = program->new_tree_node<Assign>(@$, $1, $3);
    }
  | Expression '.' OBJECTID '(' OptionalExpressions ')' {
      $$ = program->new_tree_node<Dispatch>(@$, $1, $3, nullptr, $5);
    }
  | Expression '@' TYPEID '.' OBJECTID '(' OptionalExpressions ')' {
      $$ = program->new_tree_node<Dispatch>(@$, $1, $3, $5, $7);
    }
  | OBJECTID '(' OptionalExpressions ')' {
      $$ = program->new_tree_node<Dispatch>(@$, nullptr, $1, nullptr, $3);
    }
  | IF Expression THEN Expression ELSE Expression FI {
      $$ = program->new_tree_node<Conditional>(@$, $2, $4, $6);
    }
  | WHILE Expression LOOP Expression POOL {
      $$ = program->new_tree_node<Loop>(@$, $2, $4);
    }
  | '{' ExpressionList '}' {
      $$ = program->new_tree_node<Block>(@$, $2);
    }
  | LET Definitions IN Expression %prec DEFAULT {
      $$ = program->new_tree_node<Let>(@$, $2, $4);
    }
  | CASE Expression OF Branches ESAC {
      $$ = program->new_tree_node<Case>(@$, $2, $4);
    }
  | NEW TYPEID {
      $$ = program->new_tree_node<New>(@$, $2);
    }
  | ISVOID Expression {
      $$ = program->new_tree_node<IsVoid>(@$, $2);
    }
  | Expression '+' Expression {
      $$ = program->new_tree_node<Arithmetic>(@$, ArithmeticOperator::ADD, $1, $3);
    }
  | Expression '-' Expression {
      $$ = program->new_tree_node<Arithmetic>(@$, ArithmeticOperator::SUB, $1, $3);
    }
  | Expression '*' Expression {
      $$ = program->new_tree_node<Arithmetic>(@$, ArithmeticOperator::MUL, $1, $3);
    }
  | Expression '/' Expression {
      $$ = program->new_tree_node<Arithmetic>(@$, ArithmeticOperator::DIV, $1, $3);
    }
  | '~' Expression {
      $$ = program->new_tree_node<Complement>(@$, $2);
    }
  | Expression '<' Expression {
      $$ = program->new_tree_node<Comparison>(@$, ComparisonOperator::LT, $1, $3);
    }
  | Expression "<=" Expression {
      $$ = program->new_tree_node<Comparison>(@$, ComparisonOperator::LE, $1, $3);
    }
  | Expression '=' Expression {
      $$ = program->new_tree_node<Comparison>(@$, ComparisonOperator::EQ, $1, $3);
    }
  | NOT Expression {
      $$ = program->new_tree_node<Not>(@$, $2);
    }
  | '(' Expression ')' {
      $$ = $2;
    }
  | OBJECTID {
      $$ = program->new_tree_node<Object>(@$, $1);
    }
  | NUMBER {
      $$ = program->new_tree_node<Integer>(@$, $1);
    }
  | STRING {
      $$ = program->new_tree_node<String>(@$, $1);
    }
  | TRUE {
      $$ = program->new_tree_node<Boolean>(@$, true);
    }
  | FALSE {
      $$ = program->new_tree_node<Boolean>(@$, false);
    }
  ;

OptionalExpressions:
    %empty {
      $$ = {};
    }
  | Expressions {
      $$ = std::move($1);
    }
  ;

Expressions:
    Expression {
      $$ = std::vector<Expression *>();
      $$.push_back($1);
    }
  | Expressions ',' Expression {
      $$ = std::move($1);
      $$.push_back($3);
    }
  ;

ExpressionList:
    Expression ';' {
      $$ = std::vector<Expression *>();
      $$.push_back($1);
    }
  | ExpressionList Expression ';' {
      $$ = std::move($1);
      $$.push_back($2);
    }
  ;

Definitions:
    Definition {
      $$ = std::vector<Definition *>();
      $$.push_back($1);
    }
  | Definitions ',' Definition {
      $$ = std::move($1);
      $$.push_back($3);
    }
  ;

Definition:
    OBJECTID ':' TYPEID {
      $$ = program->new_tree_node<Definition>(@$, $1, $3);
    }
  | OBJECTID ':' TYPEID "<-" Expression {
      $$ = program->new_tree_node<Definition>(@$, $1, $3, $5);
    }
  ;

Branches:
    Branch {
      $$ = std::vector<Branch *>();
      $$.push_back($1);
    }
  | Branches Branch {
      $$ = std::move($1);
      $$.push_back($2);
    }
  ;

Branch:
    OBJECTID ':' TYPEID "=>" Expression ';' {
      $$ = program->new_tree_node<Branch>(@$, $1, $3, $5);
    }
  ;

%%

void yy::parser::error(const location_type &loc, const std::string &msg) {
  std::cerr << program->getName() << ":" << loc << ": " << msg << std::endl;
}

void yy::parser::report_syntax_error(const context &yyctx) const {
  const value_type &yylval = yyctx.lookahead().value;

  std::cerr << program->getName()
            << ":"
            << yyctx.location()
            << ": syntax error at or near "
            << yyctx.lookahead().name();

  switch (yyctx.token()) {
    case symbol_kind_type::S_TRUE:
      std::cerr << " = true";
      break;
    case symbol_kind_type::S_FALSE:
      std::cerr << " = false";
      break;
    case symbol_kind_type::S_NUMBER:
      std::cerr << " = " << yylval.as<int>();
      break;
    case symbol_kind_type::S_STRING:
      std::cerr << " = \"";
      print_escaped_string(std::cerr, yylval.as<std::string>().c_str());
      std::cerr << "\"";
      break;
    case symbol_kind_type::S_OBJECTID:
    case symbol_kind_type::S_TYPEID:
      std::cerr << " = " << yylval.as<Symbol *>()->to_string();
      break;
    case symbol_kind_type::S_ERROR:
      std::cerr << " \"" << yylval.as<const char *>() << "\"";
      break;
  }

  std::cerr << std::endl;
}
