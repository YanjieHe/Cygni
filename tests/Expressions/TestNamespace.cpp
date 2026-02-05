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

TEST_CASE("namespace upward search for function", "[Namespace]")
{
    NamespaceFactory namespaceFactory;
    ExpressionFactory expressionFactory;
    TypeSyntaxFactory typeSyntaxFactory;

    Namespace *root = namespaceFactory.GetRoot();
    std::shared_ptr<SourceCodeFile> sourceCodeFile = std::make_shared<SourceCodeFile>("source-code-file");
    SourceRange sr(sourceCodeFile, 0, 0, 0, 0);

    /* Create namespace hierarchy: Game -> Combat -> Skills */
    namespaceFactory.Insert(root, {U"Game"});
    namespaceFactory.Insert(root, {U"Game", U"Combat"});
    namespaceFactory.Insert(root, {U"Game", U"Combat", U"Skills"});

    Namespace *nsGame = namespaceFactory.Search(root, {U"Game"});
    Namespace *nsCombat = namespaceFactory.Search(root, {U"Game", U"Combat"});
    Namespace *nsSkills = namespaceFactory.Search(root, {U"Game", U"Combat", U"Skills"});

    /* Define function "Initialize" in namespace Game */
    TypeSyntax *int32Syntax = typeSyntaxFactory.Create(sr, std::vector<std::u32string>{U"Int"}, {});
    ConstantExpression *body = expressionFactory.Create<ConstantExpression>(sr, 42, TypeCode::Int32);
    LambdaExpression *initializeInGame = expressionFactory.Create<LambdaExpression>(
        sr, U"Initialize", body, std::vector<ParameterExpression *>{}, int32Syntax, std::vector<Annotation>{});
    nsGame->Functions().AddItem(U"Initialize", initializeInGame);

    /* Define function "CalculateDamage" in namespace Combat */
    LambdaExpression *calculateDamageInCombat = expressionFactory.Create<LambdaExpression>(
        sr, U"CalculateDamage", body, std::vector<ParameterExpression *>{}, int32Syntax, std::vector<Annotation>{});
    nsCombat->Functions().AddItem(U"CalculateDamage", calculateDamageInCombat);

    /* Define function "Fireball" in namespace Skills */
    LambdaExpression *fireballInSkills = expressionFactory.Create<LambdaExpression>(
        sr, U"Fireball", body, std::vector<ParameterExpression *>{}, int32Syntax, std::vector<Annotation>{});
    nsSkills->Functions().AddItem(U"Fireball", fireballInSkills);

    /* From Skills, should find Initialize in Game (upward search: Skills -> Combat -> Game) */
    LambdaExpression *foundInitialize = namespaceFactory.SearchFunction(nsSkills, {U"Initialize"});
    REQUIRE(foundInitialize != nullptr);
    REQUIRE(foundInitialize == initializeInGame);

    /* From Skills, should find CalculateDamage in Combat (upward search: Skills -> Combat) */
    LambdaExpression *foundCalculateDamage = namespaceFactory.SearchFunction(nsSkills, {U"CalculateDamage"});
    REQUIRE(foundCalculateDamage != nullptr);
    REQUIRE(foundCalculateDamage == calculateDamageInCombat);

    /* From Skills, should find Fireball in Skills itself */
    LambdaExpression *foundFireball = namespaceFactory.SearchFunction(nsSkills, {U"Fireball"});
    REQUIRE(foundFireball != nullptr);
    REQUIRE(foundFireball == fireballInSkills);

    /* From Combat, should NOT find Fireball (it's in child Skills, not upward) */
    LambdaExpression *notFoundFireball = namespaceFactory.SearchFunction(nsCombat, {U"Fireball"});
    REQUIRE(notFoundFireball == nullptr);

    /* From Game, should NOT find CalculateDamage (it's in child Combat, not upward) */
    LambdaExpression *notFoundCalculateDamage = namespaceFactory.SearchFunction(nsGame, {U"CalculateDamage"});
    REQUIRE(notFoundCalculateDamage == nullptr);
}

TEST_CASE("namespace upward search for structure", "[Namespace]")
{
    NamespaceFactory namespaceFactory;
    ExpressionFactory expressionFactory;

    Namespace *root = namespaceFactory.GetRoot();
    std::shared_ptr<SourceCodeFile> sourceCodeFile = std::make_shared<SourceCodeFile>("source-code-file");
    SourceRange sr(sourceCodeFile, 0, 0, 0, 0);

    /* Create namespace hierarchy: Graphics -> Rendering */
    namespaceFactory.Insert(root, {U"Graphics"});
    namespaceFactory.Insert(root, {U"Graphics", U"Rendering"});

    Namespace *nsGraphics = namespaceFactory.Search(root, {U"Graphics"});
    Namespace *nsRendering = namespaceFactory.Search(root, {U"Graphics", U"Rendering"});

    /* Define structure "Color" in Graphics */
    StructureExpression *colorInGraphics = expressionFactory.Create<StructureExpression>(
        sr, std::vector<std::u32string>{U"Graphics", U"Color"},
        Cygni::Utility::OrderPreservingMap<std::u32string, TypeSyntax *>{});
    nsGraphics->Structures().AddItem(U"Color", colorInGraphics);

    /* Define structure "Shader" in Rendering */
    StructureExpression *shaderInRendering = expressionFactory.Create<StructureExpression>(
        sr, std::vector<std::u32string>{U"Graphics", U"Rendering", U"Shader"},
        Cygni::Utility::OrderPreservingMap<std::u32string, TypeSyntax *>{});
    nsRendering->Structures().AddItem(U"Shader", shaderInRendering);

    /* From Rendering, should find Color in Graphics (upward search) */
    StructureExpression *foundColor = namespaceFactory.SearchStructure(nsRendering, {U"Color"});
    REQUIRE(foundColor != nullptr);
    REQUIRE(foundColor == colorInGraphics);

    /* From Rendering, should find Shader in itself */
    StructureExpression *foundShader = namespaceFactory.SearchStructure(nsRendering, {U"Shader"});
    REQUIRE(foundShader != nullptr);
    REQUIRE(foundShader == shaderInRendering);

    /* From Graphics, should NOT find Shader (it's in child Rendering) */
    StructureExpression *notFoundShader = namespaceFactory.SearchStructure(nsGraphics, {U"Shader"});
    REQUIRE(notFoundShader == nullptr);
}

TEST_CASE("namespace upward search for global variable", "[Namespace]")
{
    NamespaceFactory namespaceFactory;
    ExpressionFactory expressionFactory;
    TypeSyntaxFactory typeSyntaxFactory;

    Namespace *root = namespaceFactory.GetRoot();
    std::shared_ptr<SourceCodeFile> sourceCodeFile = std::make_shared<SourceCodeFile>("source-code-file");
    SourceRange sr(sourceCodeFile, 0, 0, 0, 0);

    /* Create namespace hierarchy: Server -> Database */
    namespaceFactory.Insert(root, {U"Server"});
    namespaceFactory.Insert(root, {U"Server", U"Database"});

    Namespace *nsServer = namespaceFactory.Search(root, {U"Server"});
    Namespace *nsDatabase = namespaceFactory.Search(root, {U"Server", U"Database"});

    TypeSyntax *int32Syntax = typeSyntaxFactory.Create(sr, std::vector<std::u32string>{U"Int"}, {});
    ConstantExpression *initValue = expressionFactory.Create<ConstantExpression>(sr, 100, TypeCode::Int32);

    /* Define variable "MaxConnections" in Server */
    VariableDeclarationExpression *maxConnectionsInServer =
        expressionFactory.Create<VariableDeclarationExpression>(sr, U"MaxConnections", int32Syntax, initValue);
    nsServer->GlobalVariables().AddItem(U"MaxConnections", maxConnectionsInServer);

    /* Define variable "PoolSize" in Database */
    VariableDeclarationExpression *poolSizeInDatabase =
        expressionFactory.Create<VariableDeclarationExpression>(sr, U"PoolSize", int32Syntax, initValue);
    nsDatabase->GlobalVariables().AddItem(U"PoolSize", poolSizeInDatabase);

    /* From Database, should find MaxConnections in Server (upward search) */
    VariableDeclarationExpression *foundMaxConnections =
        namespaceFactory.SearchGlobalVariable(nsDatabase, {U"MaxConnections"});
    REQUIRE(foundMaxConnections != nullptr);
    REQUIRE(foundMaxConnections == maxConnectionsInServer);

    /* From Database, should find PoolSize in itself */
    VariableDeclarationExpression *foundPoolSize = namespaceFactory.SearchGlobalVariable(nsDatabase, {U"PoolSize"});
    REQUIRE(foundPoolSize != nullptr);
    REQUIRE(foundPoolSize == poolSizeInDatabase);

    /* From Server, should NOT find PoolSize (it's in child Database) */
    VariableDeclarationExpression *notFoundPoolSize = namespaceFactory.SearchGlobalVariable(nsServer, {U"PoolSize"});
    REQUIRE(notFoundPoolSize == nullptr);
}

TEST_CASE("namespace upward search with shadowing", "[Namespace]")
{
    NamespaceFactory namespaceFactory;
    ExpressionFactory expressionFactory;
    TypeSyntaxFactory typeSyntaxFactory;

    Namespace *root = namespaceFactory.GetRoot();
    std::shared_ptr<SourceCodeFile> sourceCodeFile = std::make_shared<SourceCodeFile>("source-code-file");
    SourceRange sr(sourceCodeFile, 0, 0, 0, 0);

    /* Create namespace hierarchy: Audio -> Effects */
    namespaceFactory.Insert(root, {U"Audio"});
    namespaceFactory.Insert(root, {U"Audio", U"Effects"});

    Namespace *nsAudio = namespaceFactory.Search(root, {U"Audio"});
    Namespace *nsEffects = namespaceFactory.Search(root, {U"Audio", U"Effects"});

    TypeSyntax *int32Syntax = typeSyntaxFactory.Create(sr, std::vector<std::u32string>{U"Int"}, {});
    ConstantExpression *bodyMono = expressionFactory.Create<ConstantExpression>(sr, 1, TypeCode::Int32);
    ConstantExpression *bodyStereo = expressionFactory.Create<ConstantExpression>(sr, 2, TypeCode::Int32);

    /* Define function "Process" in Audio (mono processing) */
    LambdaExpression *processInAudio = expressionFactory.Create<LambdaExpression>(
        sr, U"Process", bodyMono, std::vector<ParameterExpression *>{}, int32Syntax, std::vector<Annotation>{});
    nsAudio->Functions().AddItem(U"Process", processInAudio);

    /* Define function "Process" in Effects (stereo processing, shadows Audio's Process) */
    LambdaExpression *processInEffects = expressionFactory.Create<LambdaExpression>(
        sr, U"Process", bodyStereo, std::vector<ParameterExpression *>{}, int32Syntax, std::vector<Annotation>{});
    nsEffects->Functions().AddItem(U"Process", processInEffects);

    /* From Effects, should find Process in Effects (not Audio's) due to shadowing */
    LambdaExpression *foundProcess = namespaceFactory.SearchFunction(nsEffects, {U"Process"});
    REQUIRE(foundProcess != nullptr);
    REQUIRE(foundProcess == processInEffects);
    REQUIRE(foundProcess != processInAudio);

    /* From Audio, should find Process in Audio */
    LambdaExpression *foundProcessInAudio = namespaceFactory.SearchFunction(nsAudio, {U"Process"});
    REQUIRE(foundProcessInAudio != nullptr);
    REQUIRE(foundProcessInAudio == processInAudio);
}