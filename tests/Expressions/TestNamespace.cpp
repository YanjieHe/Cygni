#include <catch2/catch.hpp>

#include "Expressions/Namespace.hpp"

using namespace Cygni::Expressions;

TEST_CASE("namespace Geometry", "[Namespace]") {
    NamespaceFactory namespaceFactory;

    Namespace* root = namespaceFactory.Create(nullptr, U"");

    namespaceFactory.Insert(root, {U"Geometry", U"Shape"});
    namespaceFactory.Insert(root, {U"Geometry", U"Vector"});

    REQUIRE(namespaceFactory.Search(root, {U"Math"}) == nullptr);
    REQUIRE(namespaceFactory.Search(root, {U"Geometry", U"Shape"}) != nullptr);
    REQUIRE(namespaceFactory.Search(root, {U"Geometry", U"Vector"}) != nullptr);
    REQUIRE(namespaceFactory.Search(root, {U"Geometry", U"Angle"}) == nullptr);
}

TEST_CASE("namespace Universe", "[Namespace]") {
    NamespaceFactory namespaceFactory;

    Namespace* root = namespaceFactory.Create(nullptr, U"");

    namespaceFactory.Insert(root, {U"Universe", U"Galaxies"});
    namespaceFactory.Insert(root, {U"Universe", U"Particles"});
    namespaceFactory.Insert(root, {U"Universe", U"Constants"});

    REQUIRE(namespaceFactory.Search(root, {U"Space"}) == nullptr);
    REQUIRE(namespaceFactory.Search(root, {U"Universe", U"Galaxies"}) != nullptr);
    REQUIRE(namespaceFactory.Search(root, {U"Universe", U"Particles"}) != nullptr);
    REQUIRE(namespaceFactory.Search(root, {U"Universe", U"Time"}) == nullptr);

    Namespace* universeNamespace = namespaceFactory.Search(root, {U"Universe"});

    REQUIRE(namespaceFactory.Search(universeNamespace, {U"Galaxies"}) != nullptr);
    REQUIRE(namespaceFactory.Search(universeNamespace, {U"Particles"}) != nullptr);
    REQUIRE(namespaceFactory.Search(universeNamespace, {U"Time"}) == nullptr);
}

TEST_CASE("namespace Universe::Galaxies", "[Namespace]") {
    NamespaceFactory namespaceFactory;

    Namespace* root = namespaceFactory.Create(nullptr, U"");

    namespaceFactory.Insert(root, {U"Universe", U"Galaxies"});
    namespaceFactory.Insert(root, {U"Universe", U"Particles"});
    namespaceFactory.Insert(root, {U"Universe", U"Constants"});
    namespaceFactory.Insert(root, {U"Universe", U"Galaxies", U"Spiral"});

    REQUIRE(namespaceFactory.Search(root, {U"Universe", U"Galaxies", U"Spiral"}) != nullptr);
    REQUIRE(namespaceFactory.Search(root, {U"Universe", U"Galaxies", U"Elliptical"}) == nullptr);
    REQUIRE(namespaceFactory.Search(root, {U"Universe", U"Particles"}) != nullptr);
}