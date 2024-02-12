#include <catch2/catch.hpp>

#include "LexicalAnalysis/Lexer.hpp"
#include "SyntaxAnalysis/Parser.hpp"
#include "Visitors/TypeChecker.hpp"
#include "Visitors/NameLocator.hpp"
#include "Visitors/Compiler.hpp"

#include <iostream>
#include "Utility/UTF32Functions.hpp"

using namespace Cygni::LexicalAnalysis;
using namespace Cygni::SyntaxAnalysis;
using namespace Cygni::Expressions;
using namespace Cygni::Visitors;

TEST_CASE("test compiler", "[Compiler]") {
  std::shared_ptr<SourceCodeFile> sourceCodeFile =
      std::make_shared<SourceCodeFile>("source-code-file");

  Lexer lexer(sourceCodeFile, U"module A { func Square(x: Int): Int { x * x; } "
                              U"func Main(): Int { Square(3); } }");

  std::vector<Token> tokens = lexer.ReadAll();

  Parser parser(tokens, sourceCodeFile);
  parser.ParseNamespace();

  TypeChecker typeChecker(parser.GetNamespaceFactory(),
                          parser.GetExpressionFactory());

  Scope<const Type *> scope;
  typeChecker.CheckNamespace(&scope);

  Scope<NameInfo> nameInfoScope;
  NameLocator nameLocator = NameLocator(parser.GetNamespaceFactory());
  nameInfoScope.Declare(GLOBAL_VARIABLE_COUNT,
                        NameInfo(LocationKind::Global_Variable_Count, 0));
  nameInfoScope.Declare(GLOBAL_FUNCTION_COUNT,
                        NameInfo(LocationKind::Global_Function_Count, 0));
  nameInfoScope.Declare(
      GLOBAL_NATIVE_FUNCTION_COUNT,
      NameInfo(LocationKind::Global_Native_Function_Count, 0));
  nameLocator.CheckNamespace(&nameInfoScope);

  REQUIRE(nameInfoScope.Get(GLOBAL_FUNCTION_COUNT).Number() == 2);

  Compiler compiler(typeChecker, nameLocator, parser.GetNamespaceFactory());
  std::vector<flint_bytecode::GlobalVariable> globalVariables(
      nameInfoScope.Get(GLOBAL_VARIABLE_COUNT).Number());
  std::vector<flint_bytecode::Function> functions(
      nameInfoScope.Get(GLOBAL_FUNCTION_COUNT).Number());
  std::vector<flint_bytecode::NativeFunction> nativeFunctions(
      nameInfoScope.Get(GLOBAL_NATIVE_FUNCTION_COUNT).Number());
  compiler.CompileNamespace(globalVariables, functions, nativeFunctions);
  flint_bytecode::ByteCodeProgram program(globalVariables, {}, functions, {},
                                          nativeFunctions,
                                          compiler.EntryPoint());

  ByteCode byteCode;
  program.Compile(byteCode);
  byteCode.OutputToFile("square_3.bin");
}