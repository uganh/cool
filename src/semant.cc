#include <iostream>
#include <stack>
#include <unordered_map>
#include <unordered_set>

#include "cool-tree.h"
#include "cool-type.h"
#include "symtab.h"

void Program::semant(Logger &logger) const {
  static std::unordered_set<std::string> non_inheritable = {
    "SELF_TYPE", "String", "Int", "Bool"};

  TypeDict types = {
    {"Object", Type::Object_type},
    {"String", Type::String_type},
    {"IO", Type::IO_type},
    {"Int", Type::Int_type},
    {"Bool", Type::Bool_type}};

  /* Class definition checking */
  std::unordered_map<std::string, Class *> user_defined;
  for (auto &claSs : classes) {
    const std::string &name = claSs->getName();
    const std::string &base = claSs->getParentName();
    if (name == "SELF_TYPE") {
      logger.error() << "class name 'SELF_TYPE' is not allowed" << std::endl;
    } else if (types.find(name) != types.cend()) {
      /* Warning: The user-defined type has not been added at this time */
      logger.error() << "redefinition of basic class '" << name
                     << "' is not allowed" << std::endl;
    } else if (non_inheritable.count(base)) {
      logger.error() << "inherits from '" << base << "' is not allowed"
                     << std::endl;
    } else if (user_defined.find(name) != user_defined.cend()) {
      logger.error() << "class '" << name << "' was defined previously"
                     << std::endl;
    } else {
      user_defined.insert({name, claSs.get()});
    }
  }

  /* Build inheritance tree in a top-down way */
  for (auto &item : user_defined) {
    Class *claSs = item.second;
    if (types.find(claSs->getName()) == types.cend()) {
      std::vector<Class *> ancestors;
      while (true) {
        ancestors.push_back(claSs);
        if (types.find(claSs->getParentName()) != types.cend()) {
          break;
        } else {
          auto iter = user_defined.find(claSs->getParentName());
          if (iter == user_defined.cend()) {
            std::cout << "class '" << claSs->getName()
                      << "' inherits from an undefined class '"
                      << claSs->getParentName() << "'" << std::endl;
            ancestors.clear();
            break;
          } else {
            claSs = iter->second;
          }
        }
      }

      while (!ancestors.empty()) {
        ancestors.back()->semant(logger, types);
        ancestors.pop_back();
      }
    }
  }

  /* If error occurs, exit */
  logger.checkpoint();

  for (auto &claSs : classes) {
    claSs->check(logger, types);
  }

  /* Check main */
  if (types.find("Main") == types.cend()) {
    logger.error() << "class Main is not defined" << std::endl;
  }
}

void Class::semant(Logger &logger, TypeDict &types) {
  type = Type::create(name, types.find(base)->second);
  types.insert({name, type});
  logger.debug(__FILE__, __LINE__) << "define " << name << std::endl;
}

void Class::check(Logger &logger, TypeDict &types) const {
  /* Register all features */
  for (const auto &feature : features) {
    feature->semant(logger, types, type);
  }

  Symtab<const Type *> symtab;
  for (const auto &feature : features) {
    feature->check(logger, types, symtab, type);
  }
}

void Attribute::semant(Logger &logger, const TypeDict &types, Type *curr_type)
  const {
  const Type *decl_type = nullptr;
  if (type == "SELF_TYPE") {
    decl_type = Type::SELF_TYPE;
  } else {
    auto iter = types.find(type);
    if (iter == types.cend()) {
      logger.error() << "type '" << type << "' of attribute '"
                     << curr_type->getName() << "." << name << "' is undefined"
                     << std::endl;
      decl_type = Type::Object_type;
    } else {
      decl_type = iter->second;
    }
  }
  /* Attributes are visible within their initialization expression */
  curr_type->install(name, decl_type);
}

void Attribute::check(
  Logger &logger,
  TypeDict &types,
  Symtab<const Type *> &symtab,
  const Type *curr_type) const {
  const Type *decl_type = curr_type->getAttributeType(name);
  assert(decl_type);
  if (init) {
    const Type *init_type = init->check(logger, types, symtab, curr_type);
    if (!Type::isConform(curr_type, init_type, decl_type)) {
      logger.error() << "inferred type '" << init_type->getName()
                     << "' does not conform to declared type '"
                     << decl_type->getName() << "'" << std::endl;
    }
  }
}

void Method::semant(Logger &logger, const TypeDict &types, Type *curr_type)
  const {
  const Type *return_type = nullptr;
  if (type == "SELF_TYPE") {
    return_type = Type::SELF_TYPE;
  } else {
    auto iter = types.find(type);
    if (iter == types.cend()) {
      logger.error() << "the method '" << curr_type->getName() << "." << name
                     << "' has an undefined return type '" << type << "'"
                     << std::endl;
      return_type = Type::Object_type;
    } else {
      return_type = iter->second;
    }
  }

  std::vector<const Type *> parameter_types;
  for (const auto &param : params) {
    const std::string &name    = param.first;
    const std::string &type    = param.second;
    const Type *parameter_type = Type::Object_type;
    if (type == "SELF_TYPE") {
      logger.error() << "parameter '" << name << "' may not be 'SELF_TYPE'"
                     << std::endl;
    } else {
      auto iter = types.find(type);
      if (iter == types.cend()) {
        logger.error() << "parameter '" << name
                       << "' refers to an undefined type '" << type << "'"
                       << std::endl;
      } else {
        parameter_type = iter->second;
      }
    }
    parameter_types.push_back(parameter_type);
  }

  if (!curr_type->install(name, std::make_pair(return_type, parameter_types))) {
    logger.error() << "method '" << curr_type->getName() << "." << name
                   << "' is conflicts" << std::endl;
  }
}

void Method::check(
  Logger &logger,
  TypeDict &types,
  Symtab<const Type *> &symtab,
  const Type *curr_type) const {
  const FunctionType *func_type = curr_type->getFunctionType(name);
  assert(func_type);

  symtab.enterScope();
  unsigned index = 0;
  for (auto parameter_type : func_type->getParameterTypes()) {
    logger.debug(__FILE__, __LINE__) << "define " << params[index].first << ", "
                                     << parameter_type->getName() << std::endl;
    symtab.define(params[index].first, parameter_type);
    index++;
  }
  const Type *expr_type = body->check(logger, types, symtab, curr_type);
  symtab.leaveScope();

  const Type *return_type = func_type->getReturnType();
  if (!Type::isConform(curr_type, expr_type, return_type)) {
    logger.error() << "inferred type '" << expr_type->getName()
                   << "' does not conform to return type '"
                   << return_type->getName() << "'" << std::endl;
  }
}

const Type *Boolean::check(
  Logger &logger,
  const TypeDict &types,
  Symtab<const Type *> &symtab,
  const Type *curr_type) {
  return etype = Type::Bool_type;
}

const Type *Integer::check(
  Logger &logger,
  const TypeDict &types,
  Symtab<const Type *> &symtab,
  const Type *curr_type) {
  return etype = Type::Int_type;
}

const Type *String::check(
  Logger &logger,
  const TypeDict &types,
  Symtab<const Type *> &symtab,
  const Type *curr_type) {
  return etype = Type::String_type;
}

const Type *Isvoid::check(
  Logger &logger,
  const TypeDict &types,
  Symtab<const Type *> &symtab,
  const Type *curr_type) {
  expr->check(logger, types, symtab, curr_type);
  return etype = Type::Bool_type;
}

const Type *Not::check(
  Logger &logger,
  const TypeDict &types,
  Symtab<const Type *> &symtab,
  const Type *curr_type) {
  const Type *expr_type = expr->check(logger, types, symtab, curr_type);
  if (expr_type != Type::Bool_type) {
    logger.error() << "'not' operator should takes on type Bool instead of '"
                   << expr_type->getName() << "'" << std::endl;
  }
  return etype = Type::Bool_type;
}

const Type *Negate::check(
  Logger &logger,
  const TypeDict &types,
  Symtab<const Type *> &symtab,
  const Type *curr_type) {
  const Type *expr_type = expr->check(logger, types, symtab, curr_type);
  if (expr_type != Type::Int_type) {
    logger.error() << "'~' operator should takes on type Int instead of '"
                   << expr_type->getName() << "'" << std::endl;
  }
  return etype = Type::Int_type;
}

const Type *Logical::check(
  Logger &logger,
  const TypeDict &types,
  Symtab<const Type *> &symtab,
  const Type *curr_type) {
  const Type *op1_type = op1->check(logger, types, symtab, curr_type);
  const Type *op2_type = op2->check(logger, types, symtab, curr_type);
  switch (op) {
    case LT:
    case LE: {
      if (op1_type != Type::Int_type || op2_type != Type::Int_type) {
        logger.error() << "illegal logical operation: '" << op1_type->getName()
                       << "' " << opstr() << " '" << op1_type->getName() << "'"
                       << std::endl;
      }
      break;
    }
    case EQ: {
      if (
        (op1_type != Type::String_type ||
         op1_type != Type::Int_type && op1_type != Type::Bool_type) ||
        op1_type != op2_type) {
        logger.error() << "illegal logical operation: '" << op1_type->getName()
                       << "' " << opstr() << " '" << op1_type->getName() << "'"
                       << std::endl;
      }
    }
  }
  return etype = Type::Bool_type;
}

const Type *Arithmetic::check(
  Logger &logger,
  const TypeDict &types,
  Symtab<const Type *> &symtab,
  const Type *curr_type) {
  const Type *op1_type = op1->check(logger, types, symtab, curr_type);
  const Type *op2_type = op2->check(logger, types, symtab, curr_type);
  if (op1_type != Type::Int_type || op2_type != Type::Int_type) {
    logger.error() << "illegal arithmetic operation: '" << op1_type->getName()
                   << "' " << op << " '" << op1_type->getName() << "'"
                   << std::endl;
  }
  return etype = Type::Int_type;
}

const Type *Object::check(
  Logger &logger,
  const TypeDict &types,
  Symtab<const Type *> &symtab,
  const Type *curr_type) {
  /*
   * O(Id) = T
   * ---------------
   * O,M,C |- Id : T
   * */
  if (name == "self") {
    /*
     * -----------------------------
     * O,M,C |- self : SELF_TYPE_{C}
     * */
    return etype = Type::SELF_TYPE;
  } else if (auto expr_type = symtab.lookup(name)) {
    /* Lexical scope */
    return etype = *expr_type;
  } else if (auto expr_type = curr_type->getAttributeType(name)) {
    /* Class scope */
    return etype = expr_type;
  }
  logger.error() << "undeclared identifier '" << name << "'" << std::endl;
  return etype = Type::Object_type;
}

const Type *Assign::check(
  Logger &logger,
  const TypeDict &types,
  Symtab<const Type *> &symtab,
  const Type *curr_type) {
  /*
   * O(Id) = T
   * O,M,C |- e1 : T'
   * T' <= T
   * ----------------------
   * O,M,C |- Id <- e1 : T'
   * */
  const Type *left_type = left->check(logger, types, symtab, curr_type);
  const Type *expr_type = expr->check(logger, types, symtab, curr_type);
  if (!Type::isConform(curr_type, expr_type, left_type)) {
    logger.error() << "type '" << expr_type->getName()
                   << "' does not conform to type '" << left_type->getName()
                   << "'" << std::endl;
    return etype = left_type;
  }
  return etype = expr_type;
}

const Type *Dispatch::check(
  Logger &logger,
  const TypeDict &types,
  Symtab<const Type *> &symtab,
  const Type *curr_type) {
  const Type *expr_type = expr->check(logger, types, symtab, curr_type);

  const Type *real_type = nullptr;
  if (expr_type == Type::SELF_TYPE) {
    /*
     * The type T0 of the dispatch expression could be SELF_TYPE,
     * it is safe to use the class where the dispatch appears
     * */
    real_type = curr_type;
  } else {
    real_type = expr_type;
  }

  const FunctionType *func_type = real_type->getFunctionType(name);
  if (!func_type) {
    logger.error() << "dispatch on undefined method '" << real_type->getName()
                   << "." << name << "'" << std::endl;
    return etype = Type::Object_type;
  }

  auto &parameter_types = func_type->getParameterTypes();
  if (parameter_types.size() != args.size()) {
    logger.error() << "incorrect # arguments passed" << std::endl;
    return etype = Type::Object_type;
  }

  unsigned index = 0;
  for (const auto &arg : args) {
    const Type *arg_type = arg->check(logger, types, symtab, curr_type);
    if (!Type::isConform(curr_type, arg_type, parameter_types[index])) {
      logger.error() << "type '" << arg_type->getName()
                     << "' does not conform to '"
                     << parameter_types[index]->getName() << "'" << std::endl;
      return etype = Type::Object_type;
    }
    index++;
  }

  const Type *return_type = func_type->getReturnType();
  if (return_type == Type::SELF_TYPE) {
    return etype = expr_type;
  }

  return etype = return_type;
}

const Type *StaticDispatch::check(
  Logger &logger,
  const TypeDict &types,
  Symtab<const Type *> &symtab,
  const Type *curr_type) {
  auto iter = types.find(type);
  if (iter == types.cend()) {
    logger.error() << "static dispatch to undefined class '" << type << "'"
                   << std::endl;
    return etype = Type::Object_type;
  }
  const Type *base_type = iter->second;

  const FunctionType *func_type = base_type->getFunctionType(name);
  if (!func_type) {
    logger.error() << "dispatch on undefined method '" << base_type->getName()
                   << "." << name << "'" << std::endl;
    return etype = Type::Object_type;
  }

  auto &parameter_types = func_type->getParameterTypes();
  if (parameter_types.size() != args.size()) {
    logger.error() << "incorrect # arguments passed" << std::endl;
    return etype = Type::Object_type;
  }

  const Type *expr_type = expr->check(logger, types, symtab, curr_type);
  if (!Type::isConform(curr_type, expr_type, base_type)) {
    logger.error() << "type '" << expr_type->getName()
                   << "' does not conform to declared static dispatch " << type
                   << std::endl;
    return etype = Type::Object_type;
  }

  unsigned index = 0;
  for (const auto &arg : args) {
    const Type *arg_type = arg->check(logger, types, symtab, curr_type);
    if (!Type::isConform(curr_type, arg_type, parameter_types[index])) {
      logger.error() << "type '" << arg_type->getName()
                     << "' does not conform to '"
                     << parameter_types[index]->getName() << "'" << std::endl;
      return etype = Type::Object_type;
    }
    index++;
  }

  const Type *return_type = func_type->getReturnType();
  if (return_type == Type::SELF_TYPE) {
    return etype = expr_type;
  }

  return etype = return_type;
}

const Type *New::check(
  Logger &logger,
  const TypeDict &types,
  Symtab<const Type *> &symtab,
  const Type *curr_type) {
  if (type == "SELF_TYPE") {
    /*
     * --------------------------------------
     * O,M,C |- new SELF_TYPE : SELF_TYPE_{C}
     * */
    return etype = Type::SELF_TYPE;
  }
  auto iter = types.find(type);
  if (iter == types.cend()) {
    logger.error() << "try to new undefined type " << type << std::endl;
    return etype = Type::Object_type;
  }
  return etype = iter->second;
}

const Type *Conditional::check(
  Logger &logger,
  const TypeDict &types,
  Symtab<const Type *> &symtab,
  const Type *curr_type) {
  const Type *cond_type = cond->check(logger, types, symtab, curr_type);
  const Type *then_type = then->check(logger, types, symtab, curr_type);
  const Type *else_type = elSe->check(logger, types, symtab, curr_type);
  if (cond_type->getName() != "Bool") {
    logger.error() << "predicate of conditional expression must have type Bool"
                   << std::endl;
    return etype = Type::Object_type;
  }
  return etype = Type::lub(curr_type, then_type, else_type);
}

const Type *Loop::check(
  Logger &logger,
  const TypeDict &types,
  Symtab<const Type *> &symtab,
  const Type *curr_type) {
  const Type *cond_type = cond->check(logger, types, symtab, curr_type);
  if (cond_type != Type::Bool_type) {
    logger.error() << "loop condition does not have type Bool" << std::endl;
  }
  body->check(logger, types, symtab, curr_type);
  return etype = Type::Object_type;
}

const Type *Block::check(
  Logger &logger,
  const TypeDict &types,
  Symtab<const Type *> &symtab,
  const Type *curr_type) {
  const Type *expr_type = nullptr;
  for (const auto &expr : body) {
    expr_type = expr->check(logger, types, symtab, curr_type);
  }
  /* The type of the last expression */
  return etype = expr_type;
}

void Definition::check(
  Logger &logger,
  const TypeDict &types,
  Symtab<const Type *> &symtab,
  const Type *curr_type) const {
  const Type *decl_type = nullptr;
  if (type == "SELF_TYPE") {
    decl_type = Type::SELF_TYPE;
  } else {
    auto iter = types.find(type);
    if (iter == types.cend()) {
      logger.error() << "type '" << type << "' is undefined" << std::endl;
      decl_type = Type::Object_type;
    } else {
      decl_type = iter->second;
    }
  }

  if (init) {
    const Type *expr_type = init->check(logger, types, symtab, curr_type);
    if (!Type::isConform(curr_type, expr_type, decl_type)) {
      std::cout << "type '" << expr_type->getName()
                << "' does not conform to declared type '"
                << decl_type->getName() << "'" << std::endl;
    }
  }

  symtab.define(name, decl_type);
}

const Type *Let::check(
  Logger &logger,
  const TypeDict &types,
  Symtab<const Type *> &symtab,
  const Type *curr_type) {
  /* Enter a new scope */
  symtab.enterScope();
  for (const auto &def : defs) {
    def->check(logger, types, symtab, curr_type);
  }
  const Type *expr_type = body->check(logger, types, symtab, curr_type);
  symtab.leaveScope();
  return etype = expr_type;
}

std::pair<const Type *, const Type *> CaseBranch::check(
  Logger &logger,
  const TypeDict &types,
  Symtab<const Type *> &symtab,
  const Type *curr_type) const {
  const Type *decl_type = nullptr;
  if (type == "SELF_TYPE") {
    decl_type = Type::SELF_TYPE;
  } else {
    auto iter = types.find(type);
    if (iter == types.cend()) {
      logger.error() << "type '" << type << "' is not defined" << std::endl;
      decl_type = Type::Object_type;
    } else {
      decl_type = iter->second;
    }
  }

  symtab.enterScope();
  symtab.define(name, decl_type);
  const Type *expr_type = expr->check(logger, types, symtab, curr_type);
  symtab.leaveScope();

  return std::make_pair(decl_type, expr_type);
}

const Type *Case::check(
  Logger &logger,
  const TypeDict &types,
  Symtab<const Type *> &symtab,
  const Type *curr_type) {
  expr->check(logger, types, symtab, curr_type);

  const Type *case_type = nullptr;
  std::unordered_set<const Type *> decl_types;
  for (const auto &branch : cases) {
    auto item = branch->check(logger, types, symtab, curr_type);
    if (!decl_types.insert(item.first).second) {
      logger.error() << "duplicate branch type '" << item.first->getName()
                     << "' in case expression" << std::endl;
    }

    if (case_type) {
      case_type = Type::lub(curr_type, case_type, item.second);
    } else {
      case_type = item.second;
    }
  }

  return etype = case_type;
}

/*
 * A use of SELF_TYPE always refers to any subtype of the
 * current class. The exception is the type checking of
 * dispatch. The method return type of SELF_TYPE might have
 * nothing to do with the current class.
 * */
