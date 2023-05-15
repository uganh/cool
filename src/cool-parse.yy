%require "3.2"
%language "C++"

%expect 0

%define api.value.type variant

%parse-param {LexState &lexer} {ASTContext &context}

%code requires {
#include "cool-tree.h"
#include "strtab.h"

class LexState;
}

%code provides {
#define TOKID(name) yy::parser::token::name
}

%code {
#include "cool-lex.h"

#include <iostream>

#define yylex(yylval_ptr) lexer.lex(yylval_ptr)
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

%token <long> NUMBER
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
    Class
  | Classes Class
  ;

Class:
    CLASS TYPEID '{' Features '}' ';' {
      $$ = context.new_tree_node<Class>($2, nullptr, std::move($4));
    }
  | CLASS TYPEID INHERITS TYPEID '{' Features '}' ';' {
      $$ = context.new_tree_node<Class>($2, $4, std::move($6));
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
      $$ = context.new_tree_node<Method>($1, std::move($3), $6, $8);
    }
  | OBJECTID ':' TYPEID ';' {
      $$ = context.new_tree_node<Attribute>($1, $3);
    }
  | OBJECTID ':' TYPEID "<-" Expression ';' {
      $$ = context.new_tree_node<Attribute>($1, $3, $5);
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
      $$ = context.new_tree_node<Formal>($1, $3);
    }
  ;

Expression:
    OBJECTID "<-" Expression {
      $$ = context.new_tree_node<Assign>($1, $3);
    }
  | Expression '.' OBJECTID '(' OptionalExpressions ')' {
      $$ = context.new_tree_node<Dispatch>($1, $3, nullptr, $5);
    }
  | Expression '@' TYPEID '.' OBJECTID '(' OptionalExpressions ')' {
      $$ = context.new_tree_node<Dispatch>($1, $3, $5, $7);
    }
  | OBJECTID '(' OptionalExpressions ')' {
      $$ = context.new_tree_node<Dispatch>(nullptr, $1, nullptr, $3);
    }
  | IF Expression THEN Expression ELSE Expression FI {
      $$ = context.new_tree_node<Conditional>($2, $4, $6);
    }
  | WHILE Expression LOOP Expression POOL {
      $$ = context.new_tree_node<Loop>($2, $4);
    }
  | '{' ExpressionList '}' {
      $$ = context.new_tree_node<Block>($2);
    }
  | LET Definitions IN Expression %prec DEFAULT {
      $$ = context.new_tree_node<Let>($2, $4);
    }
  | CASE Expression OF Branches ESAC {
      $$ = context.new_tree_node<Case>($2, $4);
    }
  | NEW TYPEID {
      $$ = context.new_tree_node<New>($2);
    }
  | ISVOID Expression {
      $$ = context.new_tree_node<IsVoid>($2);
    }
  | Expression '+' Expression {
      $$ = context.new_tree_node<Arithmetic>(ArithmeticOperator::ADD, $1, $3);
    }
  | Expression '-' Expression {
      $$ = context.new_tree_node<Arithmetic>(ArithmeticOperator::SUB, $1, $3);
    }
  | Expression '*' Expression {
      $$ = context.new_tree_node<Arithmetic>(ArithmeticOperator::MUL, $1, $3);
    }
  | Expression '/' Expression {
      $$ = context.new_tree_node<Arithmetic>(ArithmeticOperator::DIV, $1, $3);
    }
  | '~' Expression {
      $$ = context.new_tree_node<Complement>($2);
    }
  | Expression '<' Expression {
      $$ = context.new_tree_node<Comparison>(ComparisonOperator::LT, $1, $3);
    }
  | Expression "<=" Expression {
      $$ = context.new_tree_node<Comparison>(ComparisonOperator::LE, $1, $3);
    }
  | Expression '=' Expression {
      $$ = context.new_tree_node<Comparison>(ComparisonOperator::EQ, $1, $3);
    }
  | NOT Expression {
      $$ = context.new_tree_node<Not>($2);
    }
  | '(' Expression ')' {
      $$ = $2;
    }
  | OBJECTID {
      $$ = context.new_tree_node<Object>($1);
    }
  | NUMBER {
      $$ = context.new_tree_node<Integer>($1);
    }
  | STRING {
      $$ = context.new_tree_node<String>($1);
    }
  | TRUE {
      $$ = context.new_tree_node<Boolean>(true);
    }
  | FALSE {
      $$ = context.new_tree_node<Boolean>(false);
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
      $$ = context.new_tree_node<Definition>($1, $3);
    }
  | OBJECTID ':' TYPEID "<-" Expression {
      $$ = context.new_tree_node<Definition>($1, $3, $5);
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
      $$ = context.new_tree_node<Branch>($1, $3, $5);
    }
  ;

%%

void yy::parser::error(const std::string &msg) {
  std::cerr << msg << std::endl;
}
