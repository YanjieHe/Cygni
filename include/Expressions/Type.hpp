#ifndef CYGNI_EXPRESSIONS_TYPE_HPP
#define CYGNI_EXPRESSIONS_TYPE_HPP
#include <vector>

namespace Cygni {
namespace Expressions {

enum class TypeCode {
  Array = 0,
  Boolean = 1,
  Callable = 2,
  Char = 3,
  Empty = 4,
  Float32 = 5,
  Float64 = 6,
  Int32 = 7,
  Int64 = 8,
  String = 9,
  Union = 10,
  Unknown = 11
};

class Type {
public:
  virtual TypeCode GetTypeCode() const = 0;
};

class EmptyType : public Type {
public:
  EmptyType() {}

  TypeCode GetTypeCode() const override { return TypeCode::Empty; }
};

class UnknownType : public Type {
public:
  UnknownType() {}

  TypeCode GetTypeCode() const override { return TypeCode::Unknown; }
};

class Int32Type : public Type {
public:
  Int32Type() {}

  TypeCode GetTypeCode() const override { return TypeCode::Int32; }
};

class Int64Type : public Type {
public:
  Int64Type() {}

  TypeCode GetTypeCode() const override { return TypeCode::Int64; }
};

class Float32Type : public Type {
public:
  Float32Type() {}

  TypeCode GetTypeCode() const override { return TypeCode::Float32; }
};

class Float64Type : public Type {
public:
  Float64Type() {}

  TypeCode GetTypeCode() const override { return TypeCode::Float64; }
};

class BooleanType : public Type {
public:
  BooleanType() {}

  TypeCode GetTypeCode() const override { return TypeCode::Boolean; }
};

class CharType : public Type {
public:
  CharType() {}

  TypeCode GetTypeCode() const override { return TypeCode::Char; }
};

class StringType : public Type {
public:
  StringType() {}

  TypeCode GetTypeCode() const override { return TypeCode::String; }
};

class ArrayType : public Type {
private:
  const Type *elementType;

public:
  explicit ArrayType(const Type *elementType) : elementType{elementType} {}

  TypeCode GetTypeCode() const override { return TypeCode::Array; }

  const Type *ElementType() const { return elementType; }
};

class CallableType : public Type {
private:
  std::vector<const Type *> arguments;
  const Type *returnType;

public:
  CallableType(std::vector<const Type *> arguments, const Type *returnType)
      : arguments{arguments}, returnType{returnType} {}

  TypeCode GetTypeCode() const override { return TypeCode::Callable; }

  const std::vector<const Type *> &Arguments() const { return arguments; }

  const Type *GetReturnType() const { return returnType; }
};

class UnionType : public Type {
private:
  std::vector<const Type *> types;

public:
  UnionType(std::vector<const Type *> types) : types{types} {}

  TypeCode GetTypeCode() const override { return TypeCode::Union; }

  const std::vector<const Type *> &GetTypes() const { return types; }
};

class TypeFactory {
private:
  std::vector<Type *> types;

public:
  TypeFactory() = default;
  TypeFactory(const TypeFactory &) = delete;
  ~TypeFactory();

  static Type *CreateBasicType(TypeCode typeCode);
  static bool IsBasicType(TypeCode typeCode);
  static bool AreTypesEqual(const Type *a, const Type *b);

  ArrayType *CreateArrayType(const Type *elementType);
  const Type *CreateUnionType(const Type *a, const Type *b);
  CallableType *CreateCallableType(std::vector<const Type *> arguments,
                                   const Type *returnType);

private:
  static bool AreOrderedTypesEqual(const std::vector<const Type *> &a,
                                   const std::vector<const Type *> &b);
  static bool AreUnorderedTypesEqual(const std::vector<const Type *> &a,
                                     const std::vector<const Type *> &b);

  Type *CreateType(Type *type);
};

}; /* namespace Expressions */
}; /* namespace Cygni */

#endif /* CYGNI_EXPRESSIONS_TYPE_HPP */
