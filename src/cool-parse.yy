%require "3.2"
%language "c++"
%expect 0

%define api.value.type variant
%define api.token.constructor

%code requires {
#include "cool-lex.h"
#include "cool-tree.h"
#include "strtab.h"
}

%code {
#include <cstdlib>
#include <iostream>

namespace yy {
parser::symbol_type yylex(LexState &in, Strtab &strtab);
}
}

%param {LexState &in}
%param {Strtab &strtab}
%parse-param {std::unique_ptr<Program> &program}

%nterm <std::unique_ptr<Class>> ClassDeclaration
%nterm <std::vector<std::unique_ptr<Class>>> ClassDeclarations
%nterm <std::unique_ptr<Feature>> FeatureDeclaration
%nterm <std::unique_ptr<Expression>> Expression
%nterm <std::vector<std::unique_ptr<Expression>>> ExpressionList
%nterm <std::unique_ptr<Expression>> Argument
%nterm <std::vector<std::unique_ptr<Expression>>> OptionalArguments Arguments
%nterm <std::vector<std::unique_ptr<Feature>>> FeatureDeclarations
%nterm <std::unique_ptr<Parameter>> Parameter
%nterm <std::vector<std::unique_ptr<Parameter>>> OptionalParameters Parameters
%nterm <std::unique_ptr<Definition>> Definition
%nterm <std::vector<std::unique_ptr<Definition>>> Definitions
%nterm <std::unique_ptr<CaseBranch>> CaseBranch
%nterm <std::vector<std::unique_ptr<CaseBranch>>> CaseBranchs

%token END 0
%token <const char *> ERROR

%token <long> INTEGER
/* String constants may be at most 1024 characters long. */
%token <std::string> TEXT_STRING

%token <Symbol> TYPEID
%token <Symbol> OBJECTID

%token LE "<="
%token ASSIGN "<-"
%token DARROW "=>"

/*
 * SELF_TYPE may be used in the following places: new SELF_TYPE,
 * as the return type of a method, as the declared type of a let
 * variable, or as the declared type of an attribute. No other
 * uses of SELF_TYPE are permitted.
 */

/*
 * SELF_TYPE is the type of the self parameter, which may be a
 * subtype of the class in which the method appears
 */

%token T_CASE
%token T_CLASS
%token T_ELSE
%token T_ESAC
%token T_FALSE
%token T_FI
%token T_IF
%token T_IN
%token T_INHERITS
%token T_ISVOID
%token T_LET
%token T_LOOP
%token T_NEW
%token T_NOT
%token T_OF
%token T_POOL
%token T_THEN
%token T_TRUE
%token T_WHILE

/*
 * The lowest priority is used for trivial shift-reduce conflicts
 * resolution.
 */
%precedence DEFAULT

%right "<-"
%precedence T_NOT
%nonassoc "<=" '<' '='
%left '+' '-'
%left '*' '/'
%precedence T_ISVOID
%precedence '~'
%precedence '@'
%precedence '.'

/*
 * In addition to Object, Cool has four other basic classes: Int,
 * String, Bool, and IO.
 */

/*
 * To ensure type safety, there are restrictions on the
 * redefinition of inherited methods:
 *
 * If a class C inherits a method f from an ancestor class P, then
 * C may override the inherited definition of f provided the number
 * of arguments, the types of the formal parameters, and the return
 * type are exactly the same in both definitions.
 */

%%

Program:
    ClassDeclarations {
      program = std::make_unique<Program>(std::move($1));
    }
  ;

ClassDeclarations:
    ClassDeclaration {
      $$ = std::vector<std::unique_ptr<Class>>();
      $$.emplace_back(std::move($1));
    }
  | ClassDeclarations ClassDeclaration {
      $$ = std::move($1);
      $$.emplace_back(std::move($2));
    }
  ;

ClassDeclaration:
    T_CLASS TYPEID '{' FeatureDeclarations '}' ';' {
      $$ = std::make_unique<Class>($2, strtab.get("Object"), std::move($4));
    }
  | T_CLASS TYPEID T_INHERITS TYPEID '{' FeatureDeclarations '}' ';' {
      $$ = std::make_unique<Class>($2, $4, std::move($6));
    }
  ;

FeatureDeclarations:
    %empty {
      $$ = std::vector<std::unique_ptr<Feature>>();
    }
  | FeatureDeclarations FeatureDeclaration {
      $$ = std::move($1);
      $$.emplace_back(std::move($2));
    }
  ;

FeatureDeclaration:
    OBJECTID ':' TYPEID ';' {
      $$ = std::make_unique<Attribute>($1, $3);
    }
  | OBJECTID ':' TYPEID "<-" Expression ';' {
      $$ = std::make_unique<Attribute>($1, $3, std::move($5));
    }
  | OBJECTID '(' OptionalParameters ')' ':' TYPEID '{' Expression '}' ';' {
      $$ = std::make_unique<Method>($1, $6, std::move($3), std::move($8));
    }
  ;

OptionalParameters:
    %empty {
      $$ = std::vector<std::unique_ptr<Parameter>>();
    }
  | Parameters {
      $$ = std::move($1);
    }
  ;

Parameters:
    Parameter {
      $$ = std::vector<std::unique_ptr<Parameter>>();
      $$.emplace_back(std::move($1));
    }
  | Parameters ',' Parameter {
      $$ = std::move($1);
      $$.emplace_back(std::move($3));
    }
  ;

Parameter:
    OBJECTID ':' TYPEID {
      $$ = std::make_unique<Parameter>($1, $3);
    }
  ;

Expression:
    T_TRUE {
      $$ = std::make_unique<Boolean>(true);
    }
  | T_FALSE {
      $$ = std::make_unique<Boolean>(false);
    }
  | INTEGER {
      $$ = std::make_unique<Integer>($1);
    }
  | TEXT_STRING {
      $$ = std::make_unique<String>($1);
    }
  | OBJECTID {
      $$ = std::make_unique<Object>($1);
    }
  | '(' Expression ')' {
      $$ = std::move($2);
    }
  | OBJECTID "<-" Expression {
      $$ = std::make_unique<Assign>($1, std::move($3));
    }
  | OBJECTID '(' OptionalArguments ')' {
      auto self = std::make_unique<Object>(strtab.get("self"));
      $$ = std::make_unique<Dispatch>(std::move(self), $1, std::move($3));
    }
  | Expression '.' OBJECTID '(' OptionalArguments ')' {
      $$ = std::make_unique<Dispatch>(std::move($1), $3, std::move($5));
    }
  | Expression '@' TYPEID '.' OBJECTID '(' OptionalArguments ')' {
      $$ = std::make_unique<StaticDispatch>(std::move($1), $3, $5, std::move($7));
    }
  | T_IF Expression T_THEN Expression T_ELSE Expression T_FI {
      $$ = std::make_unique<Conditional>(std::move($2), std::move($4), std::move($6));
    }
  | T_WHILE Expression T_LOOP Expression T_POOL {
      $$ = std::make_unique<Loop>(std::move($2), std::move($4));
    }
  | '{' ExpressionList '}' {
      $$ = std::make_unique<Block>(std::move($2));
    }
  | T_LET Definitions T_IN Expression %prec DEFAULT {
      $$ = std::make_unique<Let>(std::move($2), std::move($4));
    }
  | T_CASE Expression T_OF CaseBranchs T_ESAC {
      $$ = std::make_unique<Case>(std::move($2), std::move($4));
    }
  | T_NEW TYPEID {
      $$ = std::make_unique<New>($2);
    }
  | T_ISVOID Expression {
      $$ = std::make_unique<Isvoid>(std::move($2));
    }
  | Expression '+' Expression {
      $$ = std::make_unique<Binary>(Binary::ADD, std::move($1), std::move($3));
    }
  | Expression '-' Expression {
      $$ = std::make_unique<Binary>(Binary::SUB, std::move($1), std::move($3));
    }
  | Expression '*' Expression {
      $$ = std::make_unique<Binary>(Binary::MUL, std::move($1), std::move($3));
    }
  | Expression '/' Expression {
      $$ = std::make_unique<Binary>(Binary::DIV, std::move($1), std::move($3));
    }
  | Expression '<' Expression {
      $$ = std::make_unique<Binary>(Binary::LT, std::move($1), std::move($3));
    }
  | Expression "<=" Expression {
      $$ = std::make_unique<Binary>(Binary::LE, std::move($1), std::move($3));
    }
  | Expression '=' Expression {
      $$ = std::make_unique<Binary>(Binary::EQ, std::move($1), std::move($3));
    }
  | '~' Expression {
      $$ = std::make_unique<Unary>(Unary::NEG, std::move($2));
    }
  | T_NOT Expression {
      $$ = std::make_unique<Unary>(Unary::NOT, std::move($2));
    }
  ;

ExpressionList:
    Expression ';' {
      $$ = std::vector<std::unique_ptr<Expression>>();
      $$.emplace_back(std::move($1));
    }
  | ExpressionList Expression ';' {
      $$ = std::move($1);
      $$.emplace_back(std::move($2));
    }
  ;

Definitions:
    Definition {
      $$ = std::vector<std::unique_ptr<Definition>>();
      $$.emplace_back(std::move($1));
    }
  | Definitions ',' Definition {
      $$ = std::move($1);
      $$.emplace_back(std::move($3));
    }
  ;

Definition:
    OBJECTID ':' TYPEID {
      $$ = std::make_unique<Definition>($1, $3);
    }
  | OBJECTID ':' TYPEID "<-" Expression {
      $$ = std::make_unique<Definition>($1, $3, std::move($5));
    }
  ;

CaseBranchs:
    CaseBranch {
      $$ = std::vector<std::unique_ptr<CaseBranch>>();
      $$.emplace_back(std::move($1));
    }
  | CaseBranchs CaseBranch {
      $$ = std::move($1);
      $$.emplace_back(std::move($2));
    }
  ;

CaseBranch:
    OBJECTID ':' TYPEID "=>" Expression ';' {
      $$ = std::make_unique<CaseBranch>($1, $3, std::move($5));
    }
  ;

OptionalArguments:
    %empty {
      $$ = std::vector<std::unique_ptr<Expression>>();
    }
  | Arguments {
      $$ = std::move($1);
    }
  ;

Arguments:
    Argument {
      $$ = std::vector<std::unique_ptr<Expression>>();
      $$.emplace_back(std::move($1));
    }
  | Arguments ',' Argument {
      $$ = std::move($1);
      $$.emplace_back(std::move($3));
    }
  ;

Argument:
    Expression {
      $$ = std::move($1);
    }
  ;

%%

/*
 * Basic Classes:
 *
 * Object:
 *  - abort() : Object
 *  - type_name() : String
 *  - copy() : SELF_TYPE
 *
 * IO:
 *  - out_string(x : String) : SELF_TYPE
 *  - out_int(x : Int) : SELF_TYPE
 *  - in_string() : String
 *  - in_int() : Int
 *
 * Int: default initialization is 0
 *
 * String: default initialization is ""
 *  - length() : Int
 *  - concat(s : String) : String
 *  - substr(i : Int, l : Int) : String
 *
 * Bool: default initialization is false
 */

namespace yy {
void parser::error(const std::string &msg) {
  std::cerr << msg << std::endl;
  exit(EXIT_FAILURE);
}
} // namespace yy
