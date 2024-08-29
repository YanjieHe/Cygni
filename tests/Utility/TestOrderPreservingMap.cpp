#include "Utility/OrderPreservingMap.hpp"
#include <catch2/catch.hpp>


using Cygni::Utility::OrderPreservingMap;

TEST_CASE("OrderPreservingMap Basic Operations", "[OrderPreservingMap]")
{
    OrderPreservingMap<std::string, int> map;

    SECTION("AddItem and GetItemByKey")
    {
        map.AddItem("apple", 10);
        map.AddItem("banana", 20);
        map.AddItem("cherry", 30);

        REQUIRE(map.GetItemByKey("apple") == 10);
        REQUIRE(map.GetItemByKey("banana") == 20);
        REQUIRE(map.GetItemByKey("cherry") == 30);
    }

    SECTION("GetItemByIndex")
    {
        map.AddItem("apple", 10);
        map.AddItem("banana", 20);
        map.AddItem("cherry", 30);

        REQUIRE(map.GetItemByIndex(0) == 10);
        REQUIRE(map.GetItemByIndex(1) == 20);
        REQUIRE(map.GetItemByIndex(2) == 30);
    }

    SECTION("GetIndexByKey")
    {
        map.AddItem("apple", 10);
        map.AddItem("banana", 20);
        map.AddItem("cherry", 30);

        REQUIRE(map.GetIndexByKey("apple") == 0);
        REQUIRE(map.GetIndexByKey("banana") == 1);
        REQUIRE(map.GetIndexByKey("cherry") == 2);
    }

    SECTION("GetAllKeys")
    {
        map.AddItem("apple", 10);
        map.AddItem("banana", 20);
        map.AddItem("cherry", 30);

        auto keys = map.GetAllKeys();
        REQUIRE(keys.size() == 3);
        REQUIRE(keys[0] == "apple");
        REQUIRE(keys[1] == "banana");
        REQUIRE(keys[2] == "cherry");
    }

    SECTION("GetAllItems")
    {
        map.AddItem("apple", 10);
        map.AddItem("banana", 20);
        map.AddItem("cherry", 30);

        auto items = map.GetAllItems();
        REQUIRE(items.size() == 3);
        REQUIRE(items[0] == 10);
        REQUIRE(items[1] == 20);
        REQUIRE(items[2] == 30);
    }

    SECTION("ContainsKey")
    {
        map.AddItem("apple", 10);
        map.AddItem("banana", 20);

        REQUIRE(map.ContainsKey("apple"));
        REQUIRE(map.ContainsKey("banana"));
        REQUIRE_FALSE(map.ContainsKey("cherry"));
    }
}