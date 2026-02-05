#include <catch2/catch.hpp>

#include "Expressions/Namespace.hpp"
#include "LexicalAnalysis/Lexer.hpp"

using namespace Cygni::Expressions;
using Cygni::LexicalAnalysis::SourceCodeFile;

TEST_CASE("namespace Geometry", "[Namespace]")
{
    NamespaceFactory namespaceFactory;

    Namespace *root = namespaceFactory.Create(nullptr, U"");

    namespaceFactory.Insert(root, {U"Geometry", U"Shape"});
    namespaceFactory.Insert(root, {U"Geometry", U"Vector"});

    REQUIRE(namespaceFactory.Search(root, {U"Math"}) == nullptr);
    REQUIRE(namespaceFactory.Search(root, {U"Geometry", U"Shape"}) != nullptr);
    REQUIRE(namespaceFactory.Search(root, {U"Geometry", U"Vector"}) != nullptr);
    REQUIRE(namespaceFactory.Search(root, {U"Geometry", U"Angle"}) == nullptr);
}

TEST_CASE("namespace Universe", "[Namespace]")
{
    NamespaceFactory namespaceFactory;

    Namespace *root = namespaceFactory.Create(nullptr, U"");

    namespaceFactory.Insert(root, {U"Universe", U"Galaxies"});
    namespaceFactory.Insert(root, {U"Universe", U"Particles"});
    namespaceFactory.Insert(root, {U"Universe", U"Constants"});

    REQUIRE(namespaceFactory.Search(root, {U"Space"}) == nullptr);
    REQUIRE(namespaceFactory.Search(root, {U"Universe", U"Galaxies"}) != nullptr);
    REQUIRE(namespaceFactory.Search(root, {U"Universe", U"Particles"}) != nullptr);
    REQUIRE(namespaceFactory.Search(root, {U"Universe", U"Time"}) == nullptr);

    Namespace *universeNamespace = namespaceFactory.Search(root, {U"Universe"});

    REQUIRE(namespaceFactory.Search(universeNamespace, {U"Galaxies"}) != nullptr);
    REQUIRE(namespaceFactory.Search(universeNamespace, {U"Particles"}) != nullptr);
    REQUIRE(namespaceFactory.Search(universeNamespace, {U"Time"}) == nullptr);
}

TEST_CASE("namespace Universe::Galaxies", "[Namespace]")
{
    NamespaceFactory namespaceFactory;

    Namespace *root = namespaceFactory.Create(nullptr, U"");

    namespaceFactory.Insert(root, {U"Universe", U"Galaxies"});
    namespaceFactory.Insert(root, {U"Universe", U"Particles"});
    namespaceFactory.Insert(root, {U"Universe", U"Constants"});
    namespaceFactory.Insert(root, {U"Universe", U"Galaxies", U"Spiral"});

    REQUIRE(namespaceFactory.Search(root, {U"Universe", U"Galaxies", U"Spiral"}) != nullptr);
    REQUIRE(namespaceFactory.Search(root, {U"Universe", U"Galaxies", U"Elliptical"}) == nullptr);
    REQUIRE(namespaceFactory.Search(root, {U"Universe", U"Particles"}) != nullptr);
}

TEST_CASE("namespace search a global variable", "[Namespace]")
{
    NamespaceFactory namespaceFactory;

    Namespace *root = namespaceFactory.Create(nullptr, U"");

    namespaceFactory.Insert(root, {U"Physics"});
    namespaceFactory.Insert(root, {U"Physics", U"Constants"});

    Namespace *namespaceConstants = namespaceFactory.Search(root, {U"Physics", U"Constants"});
    std::shared_ptr<SourceCodeFile> sourceCodeFile = std::make_shared<SourceCodeFile>("source-code-file");
    ConstantExpression constant(SourceRange(sourceCodeFile, 0, 0, 0, 0), double_t(299792458), TypeCode::Float64);
    TypeSyntaxFactory typeSyntaxFactory;
    TypeSyntax *float64Syntax =
        typeSyntaxFactory.Create(SourceRange(sourceCodeFile, 0, 0, 0, 0), std::vector<std::u32string>{U"Double"}, {});
    VariableDeclarationExpression varDecl(SourceRange(sourceCodeFile, 0, 0, 0, 0), U"SpeedOfLight", float64Syntax,
                                          &constant);
    namespaceConstants->GlobalVariables().AddItem(U"SpeedOfLight", &varDecl);
    VariableDeclarationExpression *speedOfLightVar =
        namespaceFactory.SearchGlobalVariable(root, {U"Physics", U"Constants", U"SpeedOfLight"});
    REQUIRE(speedOfLightVar != nullptr);
    REQUIRE(&(varDecl) == speedOfLightVar);
    REQUIRE(speedOfLightVar->GetTypeSyntax() == float64Syntax);

    VariableDeclarationExpression *nonExistentVar =
        namespaceFactory.SearchGlobalVariable(root, {U"Global", U"PlanckConstant"});
    REQUIRE(nonExistentVar == nullptr);
}

TEST_CASE("namespace search a function", "[Namespace]")
{
    NamespaceFactory namespaceFactory;

    Namespace *root = namespaceFactory.Create(nullptr, U"");

    namespaceFactory.Insert(root, {U"A", U"B", U"C"});

    Namespace *ns = namespaceFactory.Search(root, {U"A", U"B", U"C"});
    std::shared_ptr<SourceCodeFile> sourceCodeFile = std::make_shared<SourceCodeFile>("source-code-file");
    ConstantExpression constant(SourceRange(sourceCodeFile, 0, 0, 0, 0), 42, TypeCode::Int32);
    TypeSyntaxFactory typeSyntaxFactory;
    TypeSyntax *int32Syntax =
        typeSyntaxFactory.Create(SourceRange(sourceCodeFile, 0, 0, 0, 0), std::vector<std::u32string>{U"Int"}, {});
    LambdaExpression funcDecl(SourceRange(sourceCodeFile, 0, 0, 0, 0), U"TargetFunction", &constant, {}, int32Syntax,
                              {});
    ns->Functions().AddItem(U"TargetFunction", &funcDecl);
    LambdaExpression *targetFunction = namespaceFactory.SearchFunction(root, {U"A", U"B", U"C", U"TargetFunction"});
    REQUIRE(targetFunction != nullptr);
    REQUIRE(&(funcDecl) == targetFunction);

    LambdaExpression *nonExistentFunction = namespaceFactory.SearchFunction(root, {U"A", U"B", U"NonExistentFunction"});
    REQUIRE(nonExistentFunction == nullptr);
}

TEST_CASE("namespace search a structure", "[Namespace]")
{
    NamespaceFactory namespaceFactory;

    Namespace *root = namespaceFactory.Create(nullptr, U"");

    namespaceFactory.Insert(root, {U"Universe", U"Galaxies"});
    namespaceFactory.Insert(root, {U"Universe", U"Particles"});
    namespaceFactory.Insert(root, {U"Universe", U"Constants"});
    namespaceFactory.Insert(root, {U"Universe", U"Galaxies", U"Spiral"});

    Namespace *spiralNamespace = namespaceFactory.Search(root, {U"Universe", U"Galaxies", U"Spiral"});
    REQUIRE(spiralNamespace != nullptr);
    std::shared_ptr<SourceCodeFile> sourceCodeFile = std::make_shared<SourceCodeFile>("source-code-file");
    std::vector<std::u32string> qualifiedName = {U"Universe", U"Galaxies", U"Spiral", U"SpiralAttributes"};
    StructureExpression spiralAttributes(SourceRange(sourceCodeFile, 0, 0, 0, 0), qualifiedName, {});
    spiralNamespace->Structures().AddItem(U"SpiralAttributes", &spiralAttributes);
    StructureExpression *structureDefinition = namespaceFactory.SearchStructure(root, qualifiedName);
    REQUIRE(structureDefinition != nullptr);
    REQUIRE(structureDefinition == &spiralAttributes);
    structureDefinition = namespaceFactory.SearchStructure(spiralNamespace, {U"SpiralAttributes"});
    REQUIRE(structureDefinition != nullptr);
    REQUIRE(structureDefinition == &spiralAttributes);

    structureDefinition = namespaceFactory.SearchStructure(spiralNamespace, {U"SpiralFormation"});
    REQUIRE(structureDefinition == nullptr);
}