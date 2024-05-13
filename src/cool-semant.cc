#include "cool-semant.h"
#include "cool-tree.h"
#include "cool-type.h"

#include <cassert>
#include <iostream>
#include <unordered_set>

class ScopeContext {
  std::vector<unsigned int> scopes;
  unsigned int locals;
  unsigned int maxLocals;
  Symtab<Symbol *> &symtab;

public:
  ScopeContext(Symtab<Symbol *> &symtab) : locals(0), maxLocals(0), symtab(symtab) {}

  void enterScope(unsigned int n) {
    symtab.enterScope();
    scopes.push_back(n);
    locals += n;
    if (locals > maxLocals) {
      maxLocals = locals;
    }
  }

  void leaveScope(void) {
    locals -= scopes.back();
    scopes.pop_back();
    symtab.leaveScope();
  }

  bool define(Symbol *name, Symbol *type, bool probe = false) {
    return symtab.define(name, type, probe);
  }

  bool lookup(Symbol *name, Symbol *&out_info) const {
    return symtab.lookup(name, out_info);
  }

  unsigned int getMaxLocals(void) const {
    return maxLocals;
  }
};

class ScopeContextGuard {
  ScopeContext &context;

public:
  ScopeContextGuard(ScopeContext &context, unsigned int n) : context(context) {
    context.enterScope(n);
  }

  ~ScopeContextGuard(void) noexcept {
    context.leaveScope();
  }
};

/**
 * The general form a type checking rule is:
 *
 * ...
 * ----------------
 * O, M, C |- e : T
 *
 * The rule should be read: In the type environment for objects O, methods M,
 * and containing class C, the expression e has type T. The dots above the
 * horizontal bar stand for other statements about the types of sub-expressions
 * of e. These other statements are hypotheses of the rule; if the hypotheses
 * are satisfied, then the statement below the bar is true.
 */

Symbol *Assign::typeCheckImpl(
  InheritanceTree &inheritanceTree,
  const Program *program,
  Symbol *currentType,
  ScopeContext &context) const {
  Symbol *exprType = expr->typeCheck(inheritanceTree, program, currentType, context);

  /**
   * O(id) = T
   * O,M,C |- e : T'
   * T' <= T
   * ---------------------
   * O,M,C |- id <- e : T'
   */

  if (left == Symbol::self) {
    // TODO: "Cannot assign to 'self'."
    std::cerr << program->getName()
              << ":"
              << program->getLine(this)
              << ": Cannot assign to 'self'."
              << std::endl;

    return Symbol::Object;
  }

  Symbol *leftType = nullptr;
  if (!context.lookup(left, leftType)) {
    if (const AttributeInfo *attrInfo = inheritanceTree.getAttributeInfo(currentType, left)) {
      leftType = attrInfo->attrType;
    }
  }

  if (leftType == nullptr) {
    // TODO: "Assignment to undeclared variable {left}."
    std::cerr << program->getName()
              << ":"
              << program->getLine(this)
              << ": Assignment to undeclared variable "
              << left->to_string()
              << "."
              << std::endl;
    
    return Symbol::Object;
  }

  if (!inheritanceTree.isConform(currentType, exprType, leftType)) {
    // TODO: "Type {exprType} of assigned expression does not conform to declared type {leftType} of identifier {left}."
    std::cerr << program->getName()
              << ":"
              << program->getLine(this)
              << ": Type "
              << exprType->to_string()
              << " of assigned expression does not conform to declared type "
              << leftType->to_string()
              << " of identifier "
              << left->to_string()
              << "."
              << std::endl;
  
    return Symbol::Object;
  }

  return exprType;
}

Symbol *Dispatch::typeCheckImpl(
  InheritanceTree &inheritanceTree,
  const Program *program,
  Symbol *currentType,
  ScopeContext &context) const {
  Symbol *exprType = nullptr;
  if (expr) {
    exprType = expr->typeCheck(inheritanceTree, program, currentType, context);
  } else {
    exprType = Symbol::SELF_TYPE;
  }

  std::vector<Symbol *> argTypes;
  for (Expression *arg : args) {
    argTypes.push_back(arg->typeCheck(inheritanceTree, program, currentType, context));
  }

  Symbol *dispatchType = nullptr;

  if (type) {
    /**
     * O,M,C |- e0 : T0
     * O,M,C |- e1 : T1
     * ...
     * O,M,C |- en : Tn
     * T0 <= T
     * M(T,f) = (T1',...,Tn',Tn+1')
     * Ti <= Ti' (1 <= i <= n)
     * Tn+1 = T0 if Tn+1' = SELF_TYPE else Tn+1'
     * -----------------------------------------
     * O,M,C |- e0@T.f(e1,...,en) : Tn+1
     */

    if (type == Symbol::SELF_TYPE) {
      // TODO: "Static dispatch to SELF_TYPE."
      std::cerr << program->getName()
                << ":"
                << program->getLine(this)
                << ": Static dispatch to SELF_TYPE."
                << std::endl;
    
      return Symbol::Object;
    }

    if (!inheritanceTree.isDefined(type)) {
      // TODO: "Static dispatch to undefined class {type}."
      std::cerr << program->getName()
                << ":"
                << program->getLine(this)
                << ": Static dispatch to undefined class "
                << type->to_string()
                << "."
                << std::endl;
      
      return Symbol::Object;
    }

    if (!inheritanceTree.isConform(currentType, exprType, type)) {
      // "Expression type {exprType} does not conform to declared static dispatch type {type}."
      std::cerr << program->getName()
                << ":"
                << program->getLine(this)
                << ": Expression type "
                << exprType->to_string()
                << " does not conform to declared static dispatch type "
                << type->to_string()
                << "."
                << std::endl;

      return Symbol::Object;
    }

    dispatchType = type;
  } else {
    /**
     * O,M,C |- e0 : T0
     * O,M,C |- e1 : T1
     * ...
     * O,M,C |- en : Tn
     * T0' = C if T0 = SELF_TYPE{C} else T0
     * M(T0',f) = (T1',...,Tn',Tn+1')
     * Ti <= Ti' (1 <= i <= n)
     * Tn+1 = T0 if Tn+1' = SELF_TYPE else Tn+1'
     * -----------------------------------------
     * O,M,C |- e0.f(e1,...,en) : Tn+1
     */

    if (exprType == Symbol::SELF_TYPE) {
      // The type T0 of the dispatch expression could be SELF_TYPE
      dispatchType = currentType;
    } else {
      dispatchType = exprType;
    }
  }

  if (const MethodInfo *methInfo = inheritanceTree.getMethodInfo(dispatchType, name)) {
    Symbol *retType = methInfo->methType.retType;
    const std::vector<std::pair<Symbol *, Symbol *>> &params = methInfo->methType.paramDecls;

    if (argTypes.size() != params.size()) {
      // TODO: "Method {name} called with wrong number of arguments."
      std::cerr << program->getName()
                << ":"
                << program->getLine(this)
                << ": Method "
                << name->to_string()
                << " called with wrong number of arguments."
                << std::endl;
    } else {
      for (size_t index = 0; index < argTypes.size(); index++) {
        Symbol *argType = argTypes[index];

        Symbol *paramName = params[index].first;
        Symbol *paramType = params[index].second;

        if (!inheritanceTree.isConform(currentType, argType, paramType)) {
          // TODO: "In call of method {name}, type {argType} of parameter {paramName} does not conform to declared type {paramType}."
          std::cerr << program->getName()
                    << ":"
                    << program->getLine(this)
                    << ": In call of method "
                    << name->to_string()
                    << ", type "
                    << argType->to_string()
                    << " of parameter "
                    << paramName->to_string()
                    << " does not conform to declared type "
                    << paramName->to_string()
                    << "."
                    << std::endl;
        }
      }
    }

    return retType != Symbol::SELF_TYPE ? retType : exprType;
  }

  // TODO: "Static dispatch to undefined method y."
  std::cerr << program->getName()
            << ":"
            << program->getLine(this)
            << ": Static dispatch to undefined method "
            << name->to_string()
            << "."
            << std::endl;

  return Symbol::Object;
}

Symbol *Conditional::typeCheckImpl(
  InheritanceTree &inheritanceTree,
  const Program *program,
  Symbol *currentType,
  ScopeContext &context) const {
  Symbol *predType = pred->typeCheck(inheritanceTree, program, currentType, context);
  Symbol *thenType = then->typeCheck(inheritanceTree, program, currentType, context);
  Symbol *elseType = elSe->typeCheck(inheritanceTree, program, currentType, context);

  /**
   * O,M,C |- e1 : Bool
   * O,M,C |- e2 : T2
   * O,M,C |- e3 : T3
   * ----------------------------------------------
   * O,M,C |- if e1 then e2 else e3 fi : lub(T2,T3)
   */

  if (predType != Symbol::Bool) {
    // TODO: "Predicate of 'if' does not have type Bool."
    std::cerr << program->getName()
              << ":"
              << program->getLine(this)
              << ": Predicate of 'if' does not have type Bool."
              << std::endl;
  
    return Symbol::Object;
  }

  return inheritanceTree.lub(currentType, thenType, elseType);
}

Symbol *Loop::typeCheckImpl(
  InheritanceTree &inheritanceTree,
  const Program *program,
  Symbol *currentType,
  ScopeContext &context) const {
  Symbol *predType = pred->typeCheck(inheritanceTree, program, currentType, context);
  Symbol *bodyType = body->typeCheck(inheritanceTree, program, currentType, context);

  /**
   * O,M,C |- e1 : Bool
   * O,M,C |- e2 : T2
   * ---------------------------------------
   * O,M,C |- while e1 loop e2 pool : Object
   */

  if (predType != Symbol::Bool) {
    // TODO: "Loop condition does not have type Bool."
    std::cerr << program->getName()
              << ":"
              << program->getLine(this)
              << ": Loop condition does not have type Bool."
              << std::endl;
  }

  return Symbol::Object;
}

Symbol *Block::typeCheckImpl(
  InheritanceTree &inheritanceTree,
  const Program *program,
  Symbol *currentType,
  ScopeContext &context) const {
  /**
   * O,M,C |- e1 : T1
   * ...
   * O,M,C |- en : Tn
   * ----------------
   * O,M,C |- { e1; ... en; } : Tn
   */
  
  Symbol *lastType = nullptr;
  
  for (Expression *expr : exprs) {
    lastType = expr->typeCheck(inheritanceTree, program, currentType, context);
  }

  return lastType;
}

void Definition::install(
  InheritanceTree &inheritanceTree,
  const Program *program,
  Symbol *currentType,
  ScopeContext &context) const {
  Symbol *initType = nullptr;
  if (init) {
    initType = init->typeCheck(inheritanceTree, program, currentType, context);
  }

  if (name == Symbol::SELF_TYPE) {
    // TODO: "'self' cannot be bound in a 'let' expression."
    std::cerr << program->getName()
              << ":"
              << program->getLine(this)
              << ": 'self' cannot be bound in a 'let' expression."
              << std::endl;

    return;
  }

  if (type != Symbol::SELF_TYPE && !inheritanceTree.isDefined(type)) {
    // TODO: "Class {type} of let-bound identifier {name} is undefined."
    std::cerr << program->getName()
              << ":"
              << program->getLine(this)
              << ": Class "
              << type->to_string()
              << " of let-bound identifier "
              << name->to_string()
              << " is undefined."
              << std::endl;

    bool ok = context.define(name, Symbol::Object);
    assert(ok);
  } else {
    if (init && !inheritanceTree.isConform(currentType, initType, type)) {
      // TODO: "Inferred type {initType} of initialization of {name} does not conform to identifier's declared type {type}."
      std::cerr << program->getName()
                << ":"
                << program->getLine(this)
                << ": Inferred type "
                << initType->to_string()
                << " of initialization of "
                << name->to_string()
                << " does not conform to identifier's declared type "
                << type->to_string()
                << "."
                << std::endl;
    }

    bool ok = context.define(name, type);
    assert(ok);
  }
}

Symbol *Let::typeCheckImpl(
  InheritanceTree &inheritanceTree,
  const Program *program,
  Symbol *currentType,
  ScopeContext &context) const {
  ScopeContextGuard scg(context, static_cast<unsigned int>(defs.size()));

  /**
   * O[T0/x],M,C |- e2 : T2
   * ------------------------------
   * O,M,C |- let x : T0 <- e1 in e2 : T2
   * 
   * O,M,C |- e1 : T1
   * T1 <= T0
   * O[T0/x],M,C |- e2 : T2
   * ------------------------------
   * O,M,C |- let x : T0 <- e1 in e2 : T2
   */

  for (Definition *def : defs) {
    def->install(inheritanceTree, program, currentType, context);
  }

  return body->typeCheck(inheritanceTree, program, currentType, context);
}

std::pair<Symbol *, Symbol *> Branch::doCheck(
  InheritanceTree &inheritanceTree,
  const Program *program,
  Symbol *currentType,
  ScopeContext &context) const {
  ScopeContextGuard scg(context, 1);

  Symbol *declType = nullptr;

  if (name == Symbol::self) {
    // TODO: "'self' bound in 'case'."
    std::cerr << program->getName()
              << ":"
              << program->getLine(this)
              << ": 'self' bound in 'case'."
              << std::endl;
  } else {
    if (type == Symbol::SELF_TYPE) {
      // TODO: "Identifier {name} declared with type SELF_TYPE in case branch."
      std::cerr << program->getName()
                << ":"
                << program->getLine(this)
                << ": Identifier "
                << name->to_string()
                << " declared with type SELF_TYPE in case branch."
                << std::endl;

      context.define(name, Symbol::Object);
    } else if (!inheritanceTree.isDefined(type)) {
      // TODO: "Class {type} of case branch is undefined."
      std::cerr << program->getName()
                << ":"
                << program->getLine(this)
                << ": Class "
                << type->to_string()
                << " of case branch is undefined."
                << std::endl;

      context.define(name, Symbol::Object);
    } else {
      declType = type;
      context.define(name, type);
    }
  }

  Symbol *exprType = expr->typeCheck(inheritanceTree, program, currentType, context);

  return { declType, exprType };
}

Symbol *Case::typeCheckImpl(
  InheritanceTree &inheritanceTree,
  const Program *program,
  Symbol *currentType,
  ScopeContext &context) const {
  /**
   * O,M,C |- e0 : T0
   * O[T1/x1],M,C |- e1 : T1'
   * ...
   * O[T1/xn],M,C |- e1 : Tn'
   * ----------------------------------------
   * O,M,C |- case e0 of x1 : T1 => e1; ... xn : Tn => en; esac : lub(T1',...,Tn')
   */

  expr->typeCheck(inheritanceTree, program, currentType, context);

  Symbol *type = nullptr;
  std::unordered_set<Symbol *> declTypes;

  for (Branch *branch : branches) {
    auto item = branch->doCheck(inheritanceTree, program, currentType, context);
    
    Symbol *declType = item.first;
    Symbol *exprType = item.second;

    if (!declTypes.insert(declType).second) {
      // TODO: "Duplicate branch {declType} in case statement."
      std::cerr << program->getName()
                << ":"
                << program->getLine(this)
                << ": Duplicate branch "
                << declType->to_string()
                << " in case statement."
                << std::endl;
    }

    if (type) {
      type = inheritanceTree.lub(currentType, type, exprType);
    } else {
      type = exprType;
    }
  }

  return type;
}

Symbol *New::typeCheckImpl(
  InheritanceTree &inheritanceTree,
  const Program *program,
  Symbol *currentType,
  ScopeContext &context) const {
  /**
   * -------------------------------------
   * O,M,C |- new SELF_TYPE : SELF_TYPE{C}
   */
  if (type == Symbol::SELF_TYPE) {
    return Symbol::SELF_TYPE;
  }

  /**
   * -------------------------------------
   * O,M,C |- new T : T
   */
  if (inheritanceTree.isDefined(type)) {
    return type;
  }

  // TODO: "'new' used with undefined class {type}."
  std::cerr << program->getName()
            << ":"
            << program->getLine(this)
            << ": 'new' used with undefined class "
            << type->to_string()
            << "."
            << std::endl;

  return Symbol::Object;
}

Symbol *IsVoid::typeCheckImpl(
  InheritanceTree &inheritanceTree,
  const Program *program,
  Symbol *currentType,
  ScopeContext &context) const {
  expr->typeCheck(inheritanceTree, program, currentType, context);

  /**
   * O,M,C |- e : T
   * ------------------------
   * O,M,C |- isvoid e : Bool
   */
  return Symbol::Bool;
}

Symbol *Arithmetic::typeCheckImpl(
  InheritanceTree &inheritanceTree,
  const Program *program,
  Symbol *currentType,
  ScopeContext &context) const {
  Symbol *op1Type = op1->typeCheck(inheritanceTree, program, currentType, context);
  Symbol *op2Type = op2->typeCheck(inheritanceTree, program, currentType, context);

  /**
   * O,M,C |- e1 : Int
   * O,M,C |- e2 : Int
   * op \in {ADD,SUB,MUL,DIV}
   * ------------------------
   * O,M,C |- e1 op e2 : Int
   */

  if (op1Type != Symbol::Int || op2Type != Symbol::Int) {
    const char *opStr = nullptr;

    switch (op) {
      case ArithmeticOperator::ADD: opStr = " + "; break;
      case ArithmeticOperator::SUB: opStr = " - "; break;
      case ArithmeticOperator::MUL: opStr = " * "; break;
      case ArithmeticOperator::DIV: opStr = " / "; break;
      default: assert(false); break;
    }

    // TODO: "non-Int arguments: {op1Type} < {op2Type}."
    std::cerr << program->getName()
              << ":"
              << program->getLine(this)
              << ": non-Int arguments: "
              << op1Type->to_string()
              << opStr
              << op2Type->to_string()
              << "."
              << std::endl;
    
    return Symbol::Object;
  }

  return Symbol::Int;
}

Symbol *Complement::typeCheckImpl(
  InheritanceTree &inheritanceTree,
  const Program *program,
  Symbol *currentType,
  ScopeContext &context) const {
  Symbol *exprType = expr->typeCheck(inheritanceTree, program, currentType, context);

  /**
   * O,M,C |- e : Int
   * ------------------
   * O,M,C |- ~ e : Int
   */

  if (exprType != Symbol::Int) {
    // TODO: "Argument of '~' has type {exprType} instead of Int."
    std::cerr << program->getName()
              << ":"
              << program->getLine(this)
              << ": Argument of '~' has type "
              << exprType->to_string()
              << " instead of Int."
              << std::endl;

    return Symbol::Object;
  }

  return Symbol::Int;
}

Symbol *Comparison::typeCheckImpl(
  InheritanceTree &inheritanceTree,
  const Program *program,
  Symbol *currentType,
  ScopeContext &context) const {
  Symbol *op1Type = op1->typeCheck(inheritanceTree, program, currentType, context);
  Symbol *op2Type = op2->typeCheck(inheritanceTree, program, currentType, context);

  if (op == ComparisonOperator::LT || op == ComparisonOperator::LE) {
    /**
     * O,M,C |- e1 : Int
     * O,M,C |- e2 : Int
     * op \in {LT,LE}
     * --------------------------
     * O,M,C |- e1 op e2 : Bool
     */

    if (op1Type == Symbol::Int && op2Type == Symbol::Int) {
      return Symbol::Bool;
    }

    // TODO: "non-Int arguments: {op1Type} < {op2Type}."
    std::cerr << program->getName()
              << ":"
              << program->getLine(this)
              << ": non-Int arguments: "
              << op1Type->to_string()
              << (op == ComparisonOperator::LT ? " < " : " <= ")
              << op2Type->to_string()
              << "."
              << std::endl;
  } else {
    /**
     * O,M,C |- e1 : T1
     * O,M,C |- e2 : T2
     * T1 \in {Int,String,Bool} or T2 \in {Int,String,Bool} => T1=T2
     * -------------------------------------------------------------
     * O,M,C |- e1 = e2 : Bool
     */

    if (op1Type == op2Type ||
        op1Type != Symbol::Int && op2Type != Symbol::Int &&
        op1Type != Symbol::String && op2Type != Symbol::String &&
        op1Type != Symbol::Bool && op2Type != Symbol::Bool) {
      return Symbol::Bool;
    }

    // TODO: "Illegal comparison with a basic type."
    std::cerr << program->getName()
              << ":"
              << program->getLine(this)
              << ": Illegal comparison with a basic type."
              << std::endl;
  }

  return Symbol::Object;
}

Symbol *Not::typeCheckImpl(
  InheritanceTree &inheritanceTree,
  const Program *program,
  Symbol *currentType,
  ScopeContext &context) const {
  Symbol *exprType = expr->typeCheck(inheritanceTree, program, currentType, context);

  /**
   * O,M,C |- e : Bool
   * ---------------------
   * O,M,C |- not e : Bool
   */

  if (exprType != Symbol::Bool) {
    // TODO: "Argument of 'not' has type {exprType} instead of Bool."
    std::cerr << program->getName()
              << ":"
              << program->getLine(this)
              << ": Argument of 'not' has type "
              << exprType->to_string()
              << " instead of Bool."
              << std::endl;
    
    return Symbol::Object;
    
  }

  return Symbol::Bool;
}

Symbol *Object::typeCheckImpl(
  InheritanceTree &inheritanceTree,
  const Program *program,
  Symbol *currentType,
  ScopeContext &context) const {
  /**
   * -------------------------
   * O,M,C |- self : SELF_TYPE
   */
  if (name == Symbol::self) {
    return Symbol::SELF_TYPE;
  }

  /**
   * O(id) = T
   * ---------------
   * O,M,C |- id : T
   */

  /* Lexical scope */
  Symbol *type;
  if (context.lookup(name, type)) {
    return type;
  }

  /* Class scope*/
  if (const AttributeInfo *attrInfo = inheritanceTree.getAttributeInfo(currentType, name)) {
    return attrInfo->attrType;
  }

  // TODO: "Undeclared identifier {name}."
  std::cerr << program->getName()
            << ":"
            << program->getLine(this)
            << ": Undeclared identifier "
            << name->to_string()
            << "."
            << std::endl;

  return Symbol::Object;
}

Symbol *Integer::typeCheckImpl(
  InheritanceTree &inheritanceTree,
  const Program *program,
  Symbol *currentType,
  ScopeContext &context) const {
  /**
   * i is an integer constant
   * ------------------------
   * O,M,C |- i : Int
   */
  return Symbol::Int;
}

Symbol *String::typeCheckImpl(
  InheritanceTree &inheritanceTree,
  const Program *program,
  Symbol *currentType,
  ScopeContext &context) const {
  /**
   * s is an string constant
   * -----------------------
   * O,M,C |- s : Int
   */
  return Symbol::String;
}

Symbol *Boolean::typeCheckImpl(
  InheritanceTree &inheritanceTree,
  const Program *program,
  Symbol *currentType,
  ScopeContext &context) const {
  /**
   * --------------------
   * O,M,C |- true : Bool
   * 
   * ---------------------
   * O,M,C |- false : Bool
   */
  return Symbol::Bool;
}

void Attribute::install(InheritanceTree &inheritanceTree, const Program *program, Symbol *currentType) const {
  if (type == Symbol::SELF_TYPE || inheritanceTree.isDefined(type)) {
    if (const AttributeInfo *attrInfo = inheritanceTree.getAttributeInfo(currentType, name)) {
      if (attrInfo->typeName == currentType) {
        // TODO: "Attribute {name} is multiply defined in class."
        std::cerr << program->getName()
                  << ":"
                  << program->getLine(this)
                  << ": Attribute "
                  << name->to_string()
                  << " is multiply defined in class."
                  << std::endl;
      } else {
        // TODO: "Attribute {name} is an attribute of an inherited class."
        std::cerr << program->getName()
                  << ":"
                  << program->getLine(this)
                  << ": Attribute "
                  << name->to_string()
                  << " is an attribute of an inherited class."
                  << std::endl;
      }
    } else {
      bool ok = inheritanceTree.installAttribute(currentType, name, type, init);
      assert(ok);
    }
  } else {
    // TODO: "Class {type} of attribute {name} is undefined."
    std::cerr << program->getName()
              << ":"
              << program->getLine(this)
              << ": Class "
              << type->to_string()
              << " of attribute "
              << name->to_string()
              << " is undefined."
              << std::endl;
  }
}

void Attribute::doCheck(
  InheritanceTree &inheritanceTree,
  const Program *program,
  Symbol *currentType,
  Symtab<Symbol *> &symtab) const {
  if (init) {
    /**
     * O{C}(x) = T0
     * O{C}[SELF_TYPE{C}/self],M,C |- e1 : T1
     * T1 <= T0
     * --------------------------------------
     * O{C},M,C |- x : T0 <- e1;
     */

    ScopeContext context(symtab);

    Symbol *initType = init->typeCheck(inheritanceTree, program, currentType, context);

    inheritanceTree.getAttributeInfo(currentType, name)->locals = context.getMaxLocals();

    if (type == Symbol::SELF_TYPE || inheritanceTree.isDefined(type)) {
      if (!inheritanceTree.isConform(currentType, initType, type)) {
        // TODO: "Inferred type {initType} of initialization of attribute {name} does not conform to declared type {type}."
        std::cerr << program->getName()
                  << ":"
                  << program->getLine(this)
                  << ": Inferred type "
                  << initType->to_string()
                  << " of initialization of attribute "
                  << name->to_string()
                  << " does not conform to declared type "
                  << type->to_string()
                  << "."
                  << std::endl;
      }
    }
  } else {
    /**
     * O{C}(x) = T
     * ------------------
     * O{C},M,C |- x : T;
     */
  }
}

void Method::install(InheritanceTree &inheritanceTree, const Program *program, Symbol *currentType) const {
  /**
   * 1. Check method prototype
   * 
   *  - The identifiers used in the formal parameter list must be distinct.
   */

  int errors = 0;

  // The identifiers used in the formal parameter list must be distinct.
  std::unordered_set<Symbol *> uniqueParamNames;

  std::vector<std::pair<Symbol *, Symbol *>> params;

  for (Formal *formal : formals) {
    Symbol *paramName = formal->getName();
    Symbol *paramType = formal->getType();

    if (paramType == Symbol::SELF_TYPE) {
      // TODO: "Formal parameter {paramName} cannot have type SELF_TYPE."
      std::cerr << program->getName()
                << ":"
                << program->getLine(formal)
                << ": Formal parameter "
                << paramName->to_string()
                << " cannot have type SELF_TYPE."
                << std::endl;

      errors++;
    } else if (!inheritanceTree.isDefined(paramType)) {
      // TODO: "Class {paramType} of formal parameter {paramName} is undefined."
      std::cerr << program->getName()
                << ":"
                << program->getLine(formal)
                << ": Class "
                << paramType->to_string()
                << " of formal parameter "
                << paramName->to_string()
                << " is undefined."
                << std::endl;
      
      errors++;
    }

    if (paramName == Symbol::self) {
      // TODO: "'self' cannot be the name of a formal parameter."
      std::cerr << program->getName()
                << ":"
                << program->getLine(formal)
                << ": 'self' cannot be the name of a formal parameter."
                << std::endl;

      errors++;
    } else if (uniqueParamNames.find(paramName) != uniqueParamNames.cend()) {
      // TODO: "Formal parameter {paramName} is multiply defined."
      std::cerr << program->getName()
                << ":"
                << program->getLine(formal)
                << ": Formal parameter "
                << paramName->to_string()
                << " is multiply defined."
                << std::endl;
      errors++;
    } else {
      uniqueParamNames.insert(paramName);
    }

    params.push_back({ paramName, paramType });
  }

  if (type != Symbol::SELF_TYPE && !inheritanceTree.isDefined(type)) {
    // TODO: "Undefined return type {type} in method {name}."
    std::cerr << program->getName()
              << ":"
              << program->getLine(this)
              << ": Undefined return type "
              << type->to_string()
              << " in method "
              << name->to_string()
              << "."
              << std::endl;
    
    errors++;
  }

  if (errors) {
    return;
  }

  /** 
   * 2. Check original method
   * 
   *  - To ensure type safety, there are restrictions on the redefinition of
   *    inherited methods: if a class C inherits a method f from an ancestor
   *    class P, then C may override the inherited definition of f provided the
   *    number of arguments, the types of the formal parameters, and the return
   *    type are exactly the same in both definitions.
   */

  if (const MethodInfo *methInfo = inheritanceTree.getMethodInfo(currentType, name)) {
    if (methInfo->typeName == currentType) {
      // TODO: "Method {name} is multiply defined."
      std::cerr << program->getName()
                << ":"
                << program->getLine(this)
                << ": Method "
                << name->to_string()
                << " is multiply defined."
                << std::endl;
      
      errors++;
    } else {
      Symbol *originalRetType = methInfo->methType.retType;
      const std::vector<std::pair<Symbol *, Symbol *>> &originalParams = methInfo->methType.paramDecls;

      if (originalRetType != type) {
        // TODO: "In redefined method {name}, return type {type} is different from original return type {originalRetType}."
        std::cerr << program->getName()
                  << ":"
                  << program->getLine(this)
                  << ": In redefined method "
                  << name->to_string()
                  << ", return type "
                  << type->to_string()
                  << " is different from original return type "
                  << originalRetType->to_string()
                  << "."
                  << std::endl;
        
        errors++;
      }

      if (originalParams.size() != params.size()) {
        // TODO: "Incompatible number of formal parameters in redefined method {name}."
        std::cerr << program->getName()
                  << ":"
                  << program->getLine(this)
                  << ": Incompatible number of formal parameters in redefined method "
                  << name->to_string()
                  << "."
                  << std::endl;

        errors++;
      } else {
        for (size_t index = 0; index < params.size(); index++) {
          Symbol *paramType = params[index].second;
          Symbol *originalParamType = originalParams[index].second;
          if (paramType != originalParamType) {
            // TODO: "In redefined method {name}, parameter type {paramType} is different from original type {originalParamType}."
            std::cerr << program->getName()
                      << ":"
                      << program->getLine(this)
                      << ": In redefined method "
                      << name->to_string()
                      << ", parameter type "
                      << paramType->to_string()
                      << " is different from original type "
                      << originalParamType->to_string()
                      << "."
                      << std::endl;

            errors++;
          }
        }
      }
    }
  }

  if (errors) {
    return;
  }

  /* 3. Install method to the environment */

  bool ok = inheritanceTree.installMethod(currentType, name, type, params, expr);
  assert(ok);
}

void Method::doCheck(
  InheritanceTree &inheritanceTree,
  const Program *program,
  Symbol *currentType,
  Symtab<Symbol *> &symtab) const {
  SymtabGuard<Symbol *> sg(symtab);

  /**
   * M(C,f) = (T1,...,Tn,Tn+1)
   * O{C}[SELF_TYPE{C}/self][T1/x1]...[Tn/xn],M,C |- e : T0
   * Tn+1' = SELF_TYPE{C} if Tn+1 = SELF_TYPE else Tn+1
   * T0 <= Tn+1'
   * ------------------------------------------------------
   * O{C},M,C |- f(x1 : T1, ..., xn : Tn) : T0 { e };
   */

  for (Formal *formal : formals) {
    Symbol *paramName = formal->getName();
    Symbol *paramType = formal->getType();

    if (paramType == Symbol::SELF_TYPE || !inheritanceTree.isDefined(paramType)) {
      continue;
    }

    if (paramName == Symbol::self) {
      continue;
    }

    symtab.define(paramName, paramType, true);
  }

  ScopeContext context(symtab);

  Symbol *exprType = expr->typeCheck(inheritanceTree, program, currentType, context);

  inheritanceTree.getMethodInfo(currentType, name)->locals = context.getMaxLocals();

  if (type == Symbol::SELF_TYPE || inheritanceTree.isDefined(type)) {
    if (!inheritanceTree.isConform(currentType, exprType, type)) {
      // TODO: "Inferred return type {exprType} of method {name} does not conform to declared return type {type}."
      std::cerr << program->getName()
                << ":"
                << program->getLine(this)
                << ": Inferred return type "
                << exprType->to_string()
                << " of method "
                << name->to_string()
                << " does not conform to declared return type "
                << type->to_string()
                << "."
                << std::endl;
    }
  }
}

void Class::install(InheritanceTree &inheritanceTree) const {
  bool ok = inheritanceTree.installClass(name, base);
  assert(ok);
}

void Class::doCheck(InheritanceTree &inheritanceTree, const Program *program) const {
  /* Install all features */
  for (Feature *feature : features) {
    feature->install(inheritanceTree, program, name);
  }

  Symtab<Symbol *> symtab;

  for (Feature *feature : features) {
    feature->doCheck(inheritanceTree, program, name, symtab);
  }
}

void Program::doCheck(InheritanceTree &inheritanceTree) const {
  for (Class *claSs : classes) {
    claSs->doCheck(inheritanceTree, this);
  }
}

bool semant(InheritanceTree &inheritanceTree, const std::vector<Program *> &programs) {
  int errors = 0;

  std::unordered_map<Symbol *, Class *> classTable;

  /* 1. Check class definitions */

  for (Program *program : programs) {
    for (Class *claSs : program->getClasses()) {
      Symbol *name = claSs->getName();
      Symbol *baseName = claSs->getBaseName();

      if (name == Symbol::SELF_TYPE) {
        // TODO: 
        std::cerr << program->getName()
                  << ":"
                  << program->getLine(claSs)
                  << ": Redefinition of class SELF_TYPE."
                  << std::endl;

        errors++;
        continue;
      }

      if (inheritanceTree.isDefined(name)) {
        // TODO: "Redefinition of basic class {name}."
        std::cerr << program->getName()
                  << ":"
                  << program->getLine(claSs)
                  << ": Redefinition of basic class "
                  << name->to_string()
                  << "."
                  << std::endl;
        
        errors++;
        continue;
      }

      if (baseName == Symbol::SELF_TYPE) {
        // TODO: "Class {name} cannot inherit class SELF_TYPE."
        std::cerr << program->getName()
                  << ":"
                  << program->getLine(claSs)
                  << ": Class "
                  << name->to_string()
                  << " cannot inherit class SELF_TYPE."
                  << std::endl;
        
        errors++;
        continue;
      }

      if (!inheritanceTree.isInheritable(baseName)) {
        // TODO: "Class {name} cannot inherit class {baseName}."
        std::cerr << program->getName()
                  << ":"
                  << program->getLine(claSs)
                  << ": Class "
                  << name->to_string()
                  << " cannot inherit class "
                  << baseName->to_string()
                  << "."
                  << std::endl;

        errors++;
        continue;
      }

      if (classTable.find(name) != classTable.cend()) {
        // TODO: "Class {name} was previously defined."
        std::cerr << program->getName()
                  << ":"
                  << program->getLine(claSs)
                  << ": Class "
                  << name->to_string()
                  << " was previously defined."
                  << std::endl;

        errors++;
        continue;
      }

      classTable.insert({ name, claSs });
    }
  }

  if (errors) {
    return false;
  }

  /* 2. Ensure that all base classes are defined */

  for (Program *program : programs) {
    for (Class *claSs : program->getClasses()) {
      Symbol *name = claSs->getName();
      Symbol *baseName = claSs->getBaseName();

      if (inheritanceTree.isDefined(baseName)) {
        continue;
      }

      if (classTable.find(baseName) == classTable.cend()) {
        // TODO: "Class {name} inherits from an undefined class {baseName}."
        std::cout << program->getName()
                  << ":"
                  << program->getLine(claSs)
                  << ": Class "
                  << name->to_string()
                  << " inherits from an undefined class "
                  << baseName->to_string()
                  << "."
                  << std::endl;
        
        errors++;
      }
    }
  }

  if (errors) {
    return false;
  }

  /* 3. Look at all classes and build an inheritance graph. */

  for (const auto &item : classTable) {
    Class *claSs = item.second;

    if (!inheritanceTree.isDefined(claSs->getName())) {
      std::vector<Class *> ancestors;

      while (true) {
        ancestors.push_back(claSs);

        Symbol *baseName = claSs->getBaseName();
        if (inheritanceTree.isDefined(baseName)) {
          break;
        }

        claSs = classTable.find(baseName)->second;
      }

      while (!ancestors.empty()) {
        ancestors.back()->install(inheritanceTree);
        ancestors.pop_back();
      }
    }
  }

  /*
   * 4. For each class
   * 
   *  a) Traverse the AST, gathering all visible declarations in a symbol table.
   *  b) Check each expression for type correctness.
   *  c) Annotate the AST with types.
   */

  for (Program *program : programs) {
    program->doCheck(inheritanceTree);
  }

  return true;
}
