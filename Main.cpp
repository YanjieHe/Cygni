#include <iostream>
#include <fstream>
#include "LexicalAnalysis/Lexer.hpp"
#include "SyntaxAnalysis/Parser.hpp"
#include "Visitors/TypeChecker.hpp"
#include "Visitors/NameLocator.hpp"
#include "Visitors/Compiler.hpp"
#include "Utility/UTF32Functions.hpp"
#include "CLI/CLI11.hpp"

using namespace Cygni::LexicalAnalysis;
using namespace Cygni::SyntaxAnalysis;
using namespace Cygni::Expressions;
using namespace Cygni::Visitors;
using namespace Cygni::Utility;

void Compile(std::string sourceFilePath, std::string targetFile) {

  std::shared_ptr<SourceCodeFile> sourceCodeFile =
      std::make_shared<SourceCodeFile>(sourceFilePath);

  std::ifstream file(sourceFilePath);

  if (!file.is_open()) {
    std::cerr << "Failed to open the file: " << sourceFilePath << std::endl;
  } else {
    std::string sourceCode((std::istreambuf_iterator<char>(file)),
                           (std::istreambuf_iterator<char>()));

    file.close();
    Lexer lexer(sourceCodeFile, UTF8ToUTF32(sourceCode));

    std::vector<Token> tokens = lexer.ReadAll();

    Parser parser(tokens, sourceCodeFile);
    parser.ParseNamespace();

    TypeChecker typeChecker(parser.GetNamespaceFactory());

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

    Compiler compiler(typeChecker, nameLocator, parser.GetNamespaceFactory());
    std::vector<flint_bytecode::Function> functions(
        nameInfoScope.Get(GLOBAL_FUNCTION_COUNT).Number());
    std::vector<flint_bytecode::NativeFunction> nativeFunctions(
        nameInfoScope.Get(GLOBAL_NATIVE_FUNCTION_COUNT).Number());
    compiler.CompileNamespace(functions, nativeFunctions);
    std::vector<flint_bytecode::NativeLibrary> nativeLibraries =
        compiler.GetNativeLibraries();
    flint_bytecode::ByteCodeProgram program({}, {}, functions, nativeLibraries,
                                            nativeFunctions,
                                            compiler.EntryPoint());

    ByteCode byteCode;
    program.Compile(byteCode);
    byteCode.OutputToFile(targetFile);
  }
}

int main(int argc, char **argv) {
  CLI::App app{"Cygni Compiler"};

  argv = app.ensure_utf8(argv);

  std::string input_file_path;
  std::string output_file_path;
  app.add_option("-i,--input", input_file_path,
                 "Path to the input Cygni source file.")
      ->required();
  app.add_option("-o,--output", output_file_path,
                 "Path to the output executable file.")
      ->required();

  CLI11_PARSE(app, argc, argv);

  Compile(input_file_path, output_file_path);
  return 0;
}