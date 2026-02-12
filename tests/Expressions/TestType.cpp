#include <catch2/catch.hpp>

#include "Expressions/Type.hpp"

using namespace Cygni::Expressions;

TEST_CASE("basic type equality", "[Type]")
{
    TypeFactory typeFactory;

    SECTION("same basic types are equal")
    {
        REQUIRE(TypeFactory::AreTypesEqual(TypeFactory::CreateBasicType(TypeCode::Int32),
                                           TypeFactory::CreateBasicType(TypeCode::Int32)));
        REQUIRE(TypeFactory::AreTypesEqual(TypeFactory::CreateBasicType(TypeCode::Int64),
                                           TypeFactory::CreateBasicType(TypeCode::Int64)));
        REQUIRE(TypeFactory::AreTypesEqual(TypeFactory::CreateBasicType(TypeCode::Float32),
                                           TypeFactory::CreateBasicType(TypeCode::Float32)));
        REQUIRE(TypeFactory::AreTypesEqual(TypeFactory::CreateBasicType(TypeCode::Float64),
                                           TypeFactory::CreateBasicType(TypeCode::Float64)));
        REQUIRE(TypeFactory::AreTypesEqual(TypeFactory::CreateBasicType(TypeCode::Boolean),
                                           TypeFactory::CreateBasicType(TypeCode::Boolean)));
        REQUIRE(TypeFactory::AreTypesEqual(TypeFactory::CreateBasicType(TypeCode::Char),
                                           TypeFactory::CreateBasicType(TypeCode::Char)));
        REQUIRE(TypeFactory::AreTypesEqual(TypeFactory::CreateBasicType(TypeCode::String),
                                           TypeFactory::CreateBasicType(TypeCode::String)));
        REQUIRE(TypeFactory::AreTypesEqual(TypeFactory::CreateBasicType(TypeCode::Empty),
                                           TypeFactory::CreateBasicType(TypeCode::Empty)));
    }

    SECTION("different basic types are not equal")
    {
        REQUIRE_FALSE(TypeFactory::AreTypesEqual(TypeFactory::CreateBasicType(TypeCode::Int32),
                                                 TypeFactory::CreateBasicType(TypeCode::Int64)));
        REQUIRE_FALSE(TypeFactory::AreTypesEqual(TypeFactory::CreateBasicType(TypeCode::Int32),
                                                 TypeFactory::CreateBasicType(TypeCode::Float32)));
        REQUIRE_FALSE(TypeFactory::AreTypesEqual(TypeFactory::CreateBasicType(TypeCode::Float32),
                                                 TypeFactory::CreateBasicType(TypeCode::Float64)));
        REQUIRE_FALSE(TypeFactory::AreTypesEqual(TypeFactory::CreateBasicType(TypeCode::Boolean),
                                                 TypeFactory::CreateBasicType(TypeCode::Int32)));
        REQUIRE_FALSE(TypeFactory::AreTypesEqual(TypeFactory::CreateBasicType(TypeCode::Char),
                                                 TypeFactory::CreateBasicType(TypeCode::String)));
    }
}

TEST_CASE("basic type creation and TypeCode", "[Type]")
{
    SECTION("CreateBasicType returns correct TypeCode")
    {
        REQUIRE(TypeFactory::CreateBasicType(TypeCode::Int32)->GetTypeCode() == TypeCode::Int32);
        REQUIRE(TypeFactory::CreateBasicType(TypeCode::Int64)->GetTypeCode() == TypeCode::Int64);
        REQUIRE(TypeFactory::CreateBasicType(TypeCode::Float32)->GetTypeCode() == TypeCode::Float32);
        REQUIRE(TypeFactory::CreateBasicType(TypeCode::Float64)->GetTypeCode() == TypeCode::Float64);
        REQUIRE(TypeFactory::CreateBasicType(TypeCode::Boolean)->GetTypeCode() == TypeCode::Boolean);
        REQUIRE(TypeFactory::CreateBasicType(TypeCode::Char)->GetTypeCode() == TypeCode::Char);
        REQUIRE(TypeFactory::CreateBasicType(TypeCode::String)->GetTypeCode() == TypeCode::String);
        REQUIRE(TypeFactory::CreateBasicType(TypeCode::Empty)->GetTypeCode() == TypeCode::Empty);
        REQUIRE(TypeFactory::CreateBasicType(TypeCode::Unknown)->GetTypeCode() == TypeCode::Unknown);
    }
}

TEST_CASE("IsBasicType", "[Type]")
{
    SECTION("basic types return true")
    {
        REQUIRE(TypeFactory::IsBasicType(TypeCode::Int32));
        REQUIRE(TypeFactory::IsBasicType(TypeCode::Int64));
        REQUIRE(TypeFactory::IsBasicType(TypeCode::Float32));
        REQUIRE(TypeFactory::IsBasicType(TypeCode::Float64));
        REQUIRE(TypeFactory::IsBasicType(TypeCode::Boolean));
        REQUIRE(TypeFactory::IsBasicType(TypeCode::Char));
        REQUIRE(TypeFactory::IsBasicType(TypeCode::String));
        REQUIRE(TypeFactory::IsBasicType(TypeCode::Empty));
        REQUIRE(TypeFactory::IsBasicType(TypeCode::Unknown));
    }

    SECTION("complex types return false")
    {
        REQUIRE_FALSE(TypeFactory::IsBasicType(TypeCode::Array));
        REQUIRE_FALSE(TypeFactory::IsBasicType(TypeCode::Callable));
        REQUIRE_FALSE(TypeFactory::IsBasicType(TypeCode::Structure));
        REQUIRE_FALSE(TypeFactory::IsBasicType(TypeCode::Interface));
        REQUIRE_FALSE(TypeFactory::IsBasicType(TypeCode::Union));
    }
}

TEST_CASE("array type equality", "[Type]")
{
    TypeFactory typeFactory;

    SECTION("same element type arrays are equal")
    {
        REQUIRE(TypeFactory::AreTypesEqual(typeFactory.CreateArrayType(TypeFactory::CreateBasicType(TypeCode::Int32)),
                                           typeFactory.CreateArrayType(TypeFactory::CreateBasicType(TypeCode::Int32))));
        REQUIRE(TypeFactory::AreTypesEqual(typeFactory.CreateArrayType(TypeFactory::CreateBasicType(TypeCode::Char)),
                                           typeFactory.CreateArrayType(TypeFactory::CreateBasicType(TypeCode::Char))));
    }

    SECTION("different element type arrays are not equal")
    {
        REQUIRE_FALSE(
            TypeFactory::AreTypesEqual(typeFactory.CreateArrayType(TypeFactory::CreateBasicType(TypeCode::Int32)),
                                       typeFactory.CreateArrayType(TypeFactory::CreateBasicType(TypeCode::Int64))));
        REQUIRE_FALSE(
            TypeFactory::AreTypesEqual(typeFactory.CreateArrayType(TypeFactory::CreateBasicType(TypeCode::Char)),
                                       typeFactory.CreateArrayType(TypeFactory::CreateBasicType(TypeCode::String))));
    }

    SECTION("nested arrays are equal when element types match")
    {
        auto innerArray1 = typeFactory.CreateArrayType(TypeFactory::CreateBasicType(TypeCode::Int32));
        auto innerArray2 = typeFactory.CreateArrayType(TypeFactory::CreateBasicType(TypeCode::Int32));
        auto nestedArray1 = typeFactory.CreateArrayType(innerArray1);
        auto nestedArray2 = typeFactory.CreateArrayType(innerArray2);

        REQUIRE(TypeFactory::AreTypesEqual(nestedArray1, nestedArray2));
    }

    SECTION("nested arrays with different inner types are not equal")
    {
        auto innerArray1 = typeFactory.CreateArrayType(TypeFactory::CreateBasicType(TypeCode::Int32));
        auto innerArray2 = typeFactory.CreateArrayType(TypeFactory::CreateBasicType(TypeCode::Float32));
        auto nestedArray1 = typeFactory.CreateArrayType(innerArray1);
        auto nestedArray2 = typeFactory.CreateArrayType(innerArray2);

        REQUIRE_FALSE(TypeFactory::AreTypesEqual(nestedArray1, nestedArray2));
    }

    SECTION("ArrayType returns correct TypeCode and element type")
    {
        auto arrayType = typeFactory.CreateArrayType(TypeFactory::CreateBasicType(TypeCode::String));
        REQUIRE(arrayType->GetTypeCode() == TypeCode::Array);
        REQUIRE(arrayType->ElementType()->GetTypeCode() == TypeCode::String);
    }
}

TEST_CASE("callable type equality", "[Type]")
{
    TypeFactory typeFactory;

    SECTION("same callable types are equal")
    {
        auto callable1 = typeFactory.CreateCallableType({TypeFactory::CreateBasicType(TypeCode::Int32)},
                                                        TypeFactory::CreateBasicType(TypeCode::Boolean));
        auto callable2 = typeFactory.CreateCallableType({TypeFactory::CreateBasicType(TypeCode::Int32)},
                                                        TypeFactory::CreateBasicType(TypeCode::Boolean));
        REQUIRE(TypeFactory::AreTypesEqual(callable1, callable2));
    }

    SECTION("different return types are not equal")
    {
        auto callable1 = typeFactory.CreateCallableType({TypeFactory::CreateBasicType(TypeCode::Int32)},
                                                        TypeFactory::CreateBasicType(TypeCode::Boolean));
        auto callable2 = typeFactory.CreateCallableType({TypeFactory::CreateBasicType(TypeCode::Int32)},
                                                        TypeFactory::CreateBasicType(TypeCode::Int32));
        REQUIRE_FALSE(TypeFactory::AreTypesEqual(callable1, callable2));
    }

    SECTION("different argument types are not equal")
    {
        auto callable1 = typeFactory.CreateCallableType({TypeFactory::CreateBasicType(TypeCode::Int32)},
                                                        TypeFactory::CreateBasicType(TypeCode::Boolean));
        auto callable2 = typeFactory.CreateCallableType({TypeFactory::CreateBasicType(TypeCode::Float32)},
                                                        TypeFactory::CreateBasicType(TypeCode::Boolean));
        REQUIRE_FALSE(TypeFactory::AreTypesEqual(callable1, callable2));
    }

    SECTION("different argument count are not equal")
    {
        auto callable1 = typeFactory.CreateCallableType({TypeFactory::CreateBasicType(TypeCode::Int32)},
                                                        TypeFactory::CreateBasicType(TypeCode::Boolean));
        auto callable2 = typeFactory.CreateCallableType(
            {TypeFactory::CreateBasicType(TypeCode::Int32), TypeFactory::CreateBasicType(TypeCode::Int32)},
            TypeFactory::CreateBasicType(TypeCode::Boolean));
        REQUIRE_FALSE(TypeFactory::AreTypesEqual(callable1, callable2));
    }

    SECTION("argument order matters")
    {
        auto callable1 = typeFactory.CreateCallableType(
            {TypeFactory::CreateBasicType(TypeCode::Int32), TypeFactory::CreateBasicType(TypeCode::String)},
            TypeFactory::CreateBasicType(TypeCode::Boolean));
        auto callable2 = typeFactory.CreateCallableType(
            {TypeFactory::CreateBasicType(TypeCode::String), TypeFactory::CreateBasicType(TypeCode::Int32)},
            TypeFactory::CreateBasicType(TypeCode::Boolean));
        REQUIRE_FALSE(TypeFactory::AreTypesEqual(callable1, callable2));
    }

    SECTION("zero argument callable types")
    {
        auto callable1 = typeFactory.CreateCallableType({}, TypeFactory::CreateBasicType(TypeCode::Int32));
        auto callable2 = typeFactory.CreateCallableType({}, TypeFactory::CreateBasicType(TypeCode::Int32));
        REQUIRE(TypeFactory::AreTypesEqual(callable1, callable2));
    }

    SECTION("CallableType returns correct TypeCode and members")
    {
        auto callable = typeFactory.CreateCallableType(
            {TypeFactory::CreateBasicType(TypeCode::Int32), TypeFactory::CreateBasicType(TypeCode::String)},
            TypeFactory::CreateBasicType(TypeCode::Boolean));

        REQUIRE(callable->GetTypeCode() == TypeCode::Callable);
        REQUIRE(callable->Arguments().size() == 2);
        REQUIRE(callable->Arguments()[0]->GetTypeCode() == TypeCode::Int32);
        REQUIRE(callable->Arguments()[1]->GetTypeCode() == TypeCode::String);
        REQUIRE(callable->GetReturnType()->GetTypeCode() == TypeCode::Boolean);
    }
}

TEST_CASE("structure type equality", "[Type]")
{
    TypeFactory typeFactory;

    SECTION("same structure types are equal")
    {
        Cygni::Utility::OrderPreservingMap<std::u32string, const Type *> fields1;
        fields1.AddItem(U"x", TypeFactory::CreateBasicType(TypeCode::Int32));
        fields1.AddItem(U"y", TypeFactory::CreateBasicType(TypeCode::Int32));

        Cygni::Utility::OrderPreservingMap<std::u32string, const Type *> fields2;
        fields2.AddItem(U"x", TypeFactory::CreateBasicType(TypeCode::Int32));
        fields2.AddItem(U"y", TypeFactory::CreateBasicType(TypeCode::Int32));

        auto struct1 = typeFactory.CreateStructureType({U"Point"}, fields1, {}, {});
        auto struct2 = typeFactory.CreateStructureType({U"Point"}, fields2, {}, {});

        REQUIRE(TypeFactory::AreTypesEqual(struct1, struct2));
    }

    SECTION("different names are not equal")
    {
        Cygni::Utility::OrderPreservingMap<std::u32string, const Type *> fields;
        fields.AddItem(U"x", TypeFactory::CreateBasicType(TypeCode::Int32));

        auto struct1 = typeFactory.CreateStructureType({U"PointA"}, fields, {}, {});
        auto struct2 = typeFactory.CreateStructureType({U"PointB"}, fields, {}, {});

        REQUIRE_FALSE(TypeFactory::AreTypesEqual(struct1, struct2));
    }

    SECTION("qualified names with namespaces")
    {
        Cygni::Utility::OrderPreservingMap<std::u32string, const Type *> fields;
        fields.AddItem(U"value", TypeFactory::CreateBasicType(TypeCode::Int32));

        auto struct1 = typeFactory.CreateStructureType({U"Math", U"Point"}, fields, {}, {});
        auto struct2 = typeFactory.CreateStructureType({U"Math", U"Point"}, fields, {}, {});
        auto struct3 = typeFactory.CreateStructureType({U"Graphics", U"Point"}, fields, {}, {});

        REQUIRE(TypeFactory::AreTypesEqual(struct1, struct2));
        REQUIRE_FALSE(TypeFactory::AreTypesEqual(struct1, struct3));
    }

    SECTION("StructureType returns correct members")
    {
        Cygni::Utility::OrderPreservingMap<std::u32string, const Type *> fields;
        fields.AddItem(U"name", TypeFactory::CreateBasicType(TypeCode::String));
        fields.AddItem(U"age", TypeFactory::CreateBasicType(TypeCode::Int32));

        auto structType = typeFactory.CreateStructureType({U"Person"}, fields, {}, {});
        REQUIRE(structType->GetTypeCode() == TypeCode::Structure);
        REQUIRE(structType->QualifiedName().size() == 1);
        REQUIRE(structType->QualifiedName()[0] == U"Person");
        REQUIRE(structType->Fields().GetAllKeys().size() == 2);
        REQUIRE(structType->Fields().ContainsKey(U"name"));
        REQUIRE(structType->Fields().ContainsKey(U"age"));
    }
}

TEST_CASE("structure type SetFields", "[Type]")
{
    TypeFactory typeFactory;

    SECTION("SetFields updates fields correctly")
    {
        // Create empty structure first
        auto structType = typeFactory.CreateStructureType({U"Node"}, {}, {}, {});
        REQUIRE(structType->Fields().GetAllKeys().size() == 0);

        // Update fields
        Cygni::Utility::OrderPreservingMap<std::u32string, const Type *> fields;
        fields.AddItem(U"value", TypeFactory::CreateBasicType(TypeCode::Int32));
        fields.AddItem(U"next", structType); // Self-reference

        structType->SetFields(fields);

        REQUIRE(structType->Fields().GetAllKeys().size() == 2);
        REQUIRE(structType->Fields().ContainsKey(U"value"));
        REQUIRE(structType->Fields().ContainsKey(U"next"));
        REQUIRE(structType->Fields().GetItemByKey(U"next") == structType);
    }
}

TEST_CASE("union type", "[Type]")
{
    TypeFactory typeFactory;

    SECTION("CreateUnionType creates union of different types")
    {
        auto unionType = typeFactory.CreateUnionType(TypeFactory::CreateBasicType(TypeCode::Int32),
                                                     TypeFactory::CreateBasicType(TypeCode::String));
        REQUIRE(unionType->GetTypeCode() == TypeCode::Union);
    }

    SECTION("CreateUnionType with same types returns single type")
    {
        auto result = typeFactory.CreateUnionType(TypeFactory::CreateBasicType(TypeCode::Int32),
                                                  TypeFactory::CreateBasicType(TypeCode::Int32));
        // Should return the same type, not a union
        REQUIRE(result->GetTypeCode() == TypeCode::Int32);
    }
}

TEST_CASE("different type categories are not equal", "[Type]")
{
    TypeFactory typeFactory;

    auto intType = TypeFactory::CreateBasicType(TypeCode::Int32);
    auto arrayType = typeFactory.CreateArrayType(intType);
    auto callableType = typeFactory.CreateCallableType({intType}, intType);

    Cygni::Utility::OrderPreservingMap<std::u32string, const Type *> fields;
    fields.AddItem(U"x", intType);
    auto structType = typeFactory.CreateStructureType({U"Point"}, fields, {}, {});

    SECTION("basic type vs array")
    {
        REQUIRE_FALSE(TypeFactory::AreTypesEqual(intType, arrayType));
    }

    SECTION("basic type vs callable")
    {
        REQUIRE_FALSE(TypeFactory::AreTypesEqual(intType, callableType));
    }

    SECTION("basic type vs structure")
    {
        REQUIRE_FALSE(TypeFactory::AreTypesEqual(intType, structType));
    }

    SECTION("array vs callable")
    {
        REQUIRE_FALSE(TypeFactory::AreTypesEqual(arrayType, callableType));
    }

    SECTION("array vs structure")
    {
        REQUIRE_FALSE(TypeFactory::AreTypesEqual(arrayType, structType));
    }

    SECTION("callable vs structure")
    {
        REQUIRE_FALSE(TypeFactory::AreTypesEqual(callableType, structType));
    }
}
