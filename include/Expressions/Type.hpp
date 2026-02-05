#ifndef CYGNI_EXPRESSIONS_TYPE_HPP
#define CYGNI_EXPRESSIONS_TYPE_HPP
#include "Utility/OrderPreservingMap.hpp"
#include <utility>
#include <vector>

namespace Cygni
{
namespace Expressions
{

enum class TypeCode
{
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
    Structure = 10,
    Interface = 11,
    Union = 12,
    Unknown = 13
};

class Type
{
  public:
    virtual TypeCode GetTypeCode() const = 0;
    virtual ~Type() = default;
};

template <TypeCode Code>
class BasicType : public Type
{
  public:
    BasicType() = default;
    TypeCode GetTypeCode() const override
    {
        return Code;
    }
};

using EmptyType = BasicType<TypeCode::Empty>;
using UnknownType = BasicType<TypeCode::Unknown>;
using Int32Type = BasicType<TypeCode::Int32>;
using Int64Type = BasicType<TypeCode::Int64>;
using Float32Type = BasicType<TypeCode::Float32>;
using Float64Type = BasicType<TypeCode::Float64>;
using BooleanType = BasicType<TypeCode::Boolean>;
using CharType = BasicType<TypeCode::Char>;
using StringType = BasicType<TypeCode::String>;

class ArrayType : public Type
{
  private:
    const Type *elementType;

  public:
    explicit ArrayType(const Type *elementType) : elementType{elementType}
    {
    }

    TypeCode GetTypeCode() const override
    {
        return TypeCode::Array;
    }

    const Type *ElementType() const
    {
        return elementType;
    }
};

class CallableType : public Type
{
  private:
    std::vector<const Type *> arguments;
    const Type *returnType;

  public:
    CallableType(std::vector<const Type *> arguments, const Type *returnType)
        : arguments{arguments}, returnType{returnType}
    {
    }

    TypeCode GetTypeCode() const override
    {
        return TypeCode::Callable;
    }

    const std::vector<const Type *> &Arguments() const
    {
        return arguments;
    }

    const Type *GetReturnType() const
    {
        return returnType;
    }
};

class UnionType : public Type
{
  private:
    std::vector<const Type *> types;

  public:
    UnionType(std::vector<const Type *> types) : types{types}
    {
    }

    TypeCode GetTypeCode() const override
    {
        return TypeCode::Union;
    }

    const std::vector<const Type *> &GetTypes() const
    {
        return types;
    }
};

class InterfaceType : public Type
{
  private:
    std::vector<std::u32string> qualifiedName;
    Utility::OrderPreservingMap<std::u32string, const CallableType *> methods;

  public:
    InterfaceType(const std::vector<std::u32string> &qualifiedName,
                  const Utility::OrderPreservingMap<std::u32string, const CallableType *> &methods)
        : qualifiedName{qualifiedName}, methods{methods}
    {
    }
    TypeCode GetTypeCode() const override
    {
        return TypeCode::Interface;
    }
    const std::vector<std::u32string> &QualifiedName() const
    {
        return qualifiedName;
    }
    const Utility::OrderPreservingMap<std::u32string, const CallableType *> &Methods() const
    {
        return methods;
    }
};

class StructureType : public Type
{
  private:
    std::vector<std::u32string> qualifiedName;
    Utility::OrderPreservingMap<std::u32string, const Type *> fields;
    std::vector<const InterfaceType *> interfaces;

  public:
    StructureType(const std::vector<std::u32string> &qualifiedName,
                  const Utility::OrderPreservingMap<std::u32string, const Type *> &fields,
                  const std::vector<const InterfaceType *> &interfaces)
        : qualifiedName{qualifiedName}, fields{fields}, interfaces{interfaces}
    {
    }

    TypeCode GetTypeCode() const override
    {
        return TypeCode::Structure;
    }

    const std::vector<std::u32string> &QualifiedName() const
    {
        return qualifiedName;
    }

    const Utility::OrderPreservingMap<std::u32string, const Type *> &Fields() const
    {
        return fields;
    }

    const std::vector<const InterfaceType *> &Interfaces() const
    {
        return interfaces;
    }
};

class TypeFactory
{
  private:
    std::vector<Type *> types;

  public:
    TypeFactory() = default;
    TypeFactory(const TypeFactory &) = delete;
    TypeFactory(TypeFactory &&) = delete;
    TypeFactory &operator=(const TypeFactory &) = delete;
    TypeFactory &operator=(TypeFactory &&) = delete;
    ~TypeFactory();

    static Type *CreateBasicType(TypeCode typeCode);
    static bool IsBasicType(TypeCode typeCode);
    static bool AreTypesEqual(const Type *a, const Type *b);

    ArrayType *CreateArrayType(const Type *elementType);
    const Type *CreateUnionType(const Type *a, const Type *b);
    CallableType *CreateCallableType(std::vector<const Type *> arguments, const Type *returnType);
    StructureType *CreateStructureType(const std::vector<std::u32string> &qualifiedName,
                                       const Utility::OrderPreservingMap<std::u32string, const Type *> &fields,
                                       const std::vector<const InterfaceType *> &implementedInterfaces);

  private:
    static bool AreOrderedTypesEqual(const std::vector<const Type *> &a, const std::vector<const Type *> &b);
    static bool AreUnorderedTypesEqual(const std::vector<const Type *> &a, const std::vector<const Type *> &b);

    Type *CreateType(Type *type);
};

}; /* namespace Expressions */
}; /* namespace Cygni */

#endif /* CYGNI_EXPRESSIONS_TYPE_HPP */
