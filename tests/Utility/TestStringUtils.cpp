#include "Utility/StringUtils.hpp"
#include <catch2/catch.hpp>


using Cygni::Utility::StringUtils;

TEST_CASE("StringUtils::Join", "[Join]")
{
    SECTION("Joining an empty vector results in an empty string")
    {
        std::vector<std::u32string> items;
        std::u32string result = StringUtils::Join(U", ", items);
        REQUIRE(result == U"");
    }

    SECTION("Joining a vector with one element returns the element itself")
    {
        std::vector<std::u32string> items = {U"Hello"};
        std::u32string result = StringUtils::Join(U", ", items);
        REQUIRE(result == U"Hello");
    }

    SECTION("Joining a vector with multiple elements concatenates them with the separator")
    {
        std::vector<std::u32string> items = {U"Hello", U"World"};
        std::u32string result = StringUtils::Join(U", ", items);
        REQUIRE(result == U"Hello, World");
    }

    SECTION("Joining with different separators works correctly")
    {
        std::vector<std::u32string> items = {U"A", U"B", U"C"};
        std::u32string result = StringUtils::Join(U"-", items);
        REQUIRE(result == U"A-B-C");
    }

    SECTION("Joining with special characters in the separator works correctly")
    {
        std::vector<std::u32string> items = {U"Foo", U"Bar"};
        std::u32string result = StringUtils::Join(U" & ", items);
        REQUIRE(result == U"Foo & Bar");
    }
}