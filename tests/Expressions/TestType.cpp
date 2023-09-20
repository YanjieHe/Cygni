#include <catch2/catch.hpp>

#include "Expressions/Type.hpp"

using namespace Cygni::Expressions;

TEST_CASE("type equality", "[Type]") {
  TypeFactory typeFactory;

  REQUIRE(
      TypeFactory::AreTypesEqual(typeFactory.CreateBasicType(TypeCode::Int32),
                                typeFactory.CreateBasicType(TypeCode::Int32)));
  REQUIRE(TypeFactory::AreTypesEqual(
      typeFactory.CreateBasicType(TypeCode::Float32),
      typeFactory.CreateBasicType(TypeCode::Float32)));

  REQUIRE_FALSE(TypeFactory::AreTypesEqual(
      typeFactory.CreateBasicType(TypeCode::Int32),
      typeFactory.CreateBasicType(TypeCode::Float32)));
  REQUIRE_FALSE(TypeFactory::AreTypesEqual(
      typeFactory.CreateBasicType(TypeCode::Int64),
      typeFactory.CreateBasicType(TypeCode::Float64)));

  REQUIRE(TypeFactory::AreTypesEqual(
      typeFactory.CreateArrayType(typeFactory.CreateBasicType(TypeCode::Char)),
      typeFactory.CreateArrayType(
          typeFactory.CreateBasicType(TypeCode::Char))));
}
