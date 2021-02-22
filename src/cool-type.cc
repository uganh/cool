#include "cool-type.h"

const Type Type::SELF_TYPE_private("SELF_TYPE", nullptr);
const Type *const Type::SELF_TYPE = &Type::SELF_TYPE_private;

/*
 * Object:
 *  - abort() : Object
 *  - type_name() : String
 *  - copy() : SELF_TYPE
 */
const Type Type::Object_private(
  "Object",
  nullptr,
  {},
  /* Need free */
  {{"abort", FunctionType::create(Type::Object_type)},
   {"type_name", FunctionType::create(Type::String_type)},
   {"copy", FunctionType::create(Type::SELF_TYPE)}});
const Type *const Type::Object_type = &Type::Object_private;

/*
 * IO: inherits from Object
 *  - out_string(x : String) : SELF_TYPE
 *  - out_int(x : Int) : SELF_TYPE
 *  - in_string() : String
 *  - in_int() : Int
 */
const Type Type::IO_private(
  "IO",
  Type::Object_type,
  {},
  {{"out_string", FunctionType::create(Type::SELF_TYPE, {String_type})},
   {"out_int", FunctionType::create(Type::SELF_TYPE, {Int_type})},
   {"in_string", FunctionType::create(String_type)},
   {"in_int", FunctionType::create(Int_type)}});
const Type *const Type::IO_type = &Type::IO_private;

/*
 * String: inherits from Object, default initialization is ""
 *  - length() : Int
 *  - concat(s : String) : String
 *  - substr(i : Int, l : Int) : String
 */
const Type Type::String_private(
  "String",
  Type::Object_type,
  {},
  {{"length", FunctionType::create(Int_type)},
   {"concat", FunctionType::create(String_type, {String_type})},
   {"substr", FunctionType::create(String_type, {Int_type, Int_type})}});
const Type *const Type::String_type = &Type::String_private;

/*
 * Int: inherits from Object, default initialization is 0
 */
const Type Type::Int_private("Int", Type::Object_type);
const Type *const Type::Int_type = &Type::Int_private;

/*
 * Bool: inherits from Object, default initialization is false
 */
const Type Type::Bool_private("Bool", Type::Object_type);
const Type *const Type::Bool_type = &Type::Bool_private;

const Type *Type::lub(const Type *C, const Type *T1, const Type *T2) {
  if (T1 == Type::SELF_TYPE && T2 == Type::SELF_TYPE) {
    /* lub(SELF_TYPE_{C}, SELF_TYPE_{C}) = SELF_TYPE_{C} */
    return T1;
  }

  if (T1 == Type::SELF_TYPE) {
    /* lub(SELF_TYPE_{C}, T) = lub(C, T) */
    T1 = C;
  }

  if (T2 == Type::SELF_TYPE) {
    /* lub(T, SELF_TYPE_{C}) = lub(T, C) */
    T2 = C;
  }

  while (T1->depth > T2->depth) {
    T1 = T1->getParent();
  }

  while (T1->depth < T2->depth) {
    T2 = T2->getParent();
  }

  while (T1 != T2) {
    T1 = T1->getParent();
    T2 = T2->getParent();
  }

  return T1;
}

bool Type::isConform(const Type *C, const Type *T1, const Type *T2) {
  /*
   * We write SELF_TYPE_{C} to refer to an occurrence of
   * SELF_TYPE in the body of C
   *
   * SELF_TYPE_{C} <= C
   * */

  if (T1 == Type::SELF_TYPE && T2 == Type::SELF_TYPE) {
    /*
     * SELF_TYPE_{C} <= SELF_TYPE_{C}
     *
     * In Cool we never need to compare SELF_TYPEs coming
     * from different classes
     */
    return true;
  }

  if (T1 == Type::SELF_TYPE) {
    /*
     * SELF_TYPE_{C} <= T if C <= T
     */
    return C->isConform(T2);
  }

  if (T2 == Type::SELF_TYPE) {
    /* T <= SELF_TYPE_{C} always false */
    /* Note: SELF_TYPE_{C} can denote any subtype of C */
    return false;
  }

  /* According to the rules from before */
  return T1->isConform(T2);
}