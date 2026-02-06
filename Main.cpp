#include "CLI/CLI11.hpp"
#include "LexicalAnalysis/Lexer.hpp"
#include "SyntaxAnalysis/Parser.hpp"
#include "SyntaxAnalysis/ParserException.hpp"
#include "Utility/UTF32Functions.hpp"
#include "Visitors/Compiler.hpp"
#include "Visitors/CompilationException.hpp"
#include "Visitors/ExpressionJsonSerializer.hpp"
#include "Visitors/NameLocator.hpp"
#include "Visitors/TypeChecker.hpp"
#include <fstream>
#include <iostream>

using namespace Cygni::LexicalAnalysis;
using namespace Cygni::SyntaxAnalysis;
using namespace Cygni::Expressions;
using namespace Cygni::Visitors;
using namespace Cygni::Utility;

void Compile(std::string sourceFilePath, std::string targetFilePath)
{
    std::shared_ptr<SourceCodeFile> sourceCodeFile = std::make_shared<SourceCodeFile>(sourceFilePath);

    std::ifstream file(sourceFilePath);

    if (!file.is_open())
    {
        spdlog::error("Failed to open the file: {}", sourceFilePath);
    }
    else
    {
        std::string sourceCode((std::istreambuf_iterator<char>(file)), (std::istreambuf_iterator<char>()));

        file.close();
        Lexer lexer(sourceCodeFile, UTF8ToUTF32(sourceCode));

        std::vector<Token> tokens = lexer.ReadAll();

        Parser parser(tokens, sourceCodeFile);
        parser.ParseNamespace();

        TypeChecker typeChecker(parser.GetNamespaceFactory(), parser.GetExpressionFactory());

        Scope<const Type *> scope;
        typeChecker.CheckNamespace(&scope);

        Scope<NameInfo> nameInfoScope;
        NameLocator nameLocator = NameLocator(parser.GetNamespaceFactory());
        nameLocator.InitializeSymbolCounters(&nameInfoScope);
        nameLocator.RegisterAllInfo(&nameInfoScope);
        nameLocator.CheckNamespace(&nameInfoScope);

        spdlog::info("Start compiling the program.");
        spdlog::info("Global Variable Count: {}", nameInfoScope.Get(GLOBAL_VARIABLE_COUNT).Number());
        spdlog::info("Global Function Count: {}", nameInfoScope.Get(GLOBAL_FUNCTION_COUNT).Number());
        spdlog::info("Global Structure Count: {}", nameInfoScope.Get(GLOBAL_STRUCTURE_COUNT).Number());

        Compiler compiler(typeChecker, nameLocator, parser.GetNamespaceFactory());
        std::vector<flint_bytecode::GlobalVariable> globalVariables(nameInfoScope.Get(GLOBAL_VARIABLE_COUNT).Number());
        std::vector<flint_bytecode::Function> functions(nameInfoScope.Get(GLOBAL_FUNCTION_COUNT).Number());
        std::vector<flint_bytecode::NativeFunction> nativeFunctions(
            nameInfoScope.Get(GLOBAL_NATIVE_FUNCTION_COUNT).Number());
        std::vector<flint_bytecode::StructureMeta> structures(nameInfoScope.Get(GLOBAL_STRUCTURE_COUNT).Number());
        compiler.CompileNamespace(globalVariables, functions, nativeFunctions);
        std::vector<flint_bytecode::NativeLibrary> nativeLibraries = compiler.GetNativeLibraries();
        flint_bytecode::ByteCodeProgram program(globalVariables, structures, functions, nativeLibraries,
                                                nativeFunctions, compiler.EntryPoint());

        ByteCode byteCode;
        program.Compile(byteCode);
        byteCode.OutputToFile(targetFilePath);

        spdlog::info("Compilation successful. Output written to: {}", targetFilePath);
    }
}

void TryCompile(std::string sourceFilePath, std::string targetFilePath)
{
    try
    {
        Compile(sourceFilePath, targetFilePath);
    }
    catch (LexicalException &ex)
    {
        spdlog::error(UTF32ToUTF8(ex.FormattedErrorMessage()));
    }
    catch (ParserException &ex)
    {
        SourceRange sourceRange = ex.GetSourceRange();
        spdlog::error("{}:{}:{}: {}", sourceRange.CodeFile()->FileName(), sourceRange.StartLine() + 1,
                      sourceRange.StartColumn() + 1, ex.Message());
    }
    catch (TreeException &ex)
    {
        SourceRange sourceRange = ex.GetSourceRange();
        spdlog::error("Compiler source code file: {}, line: {}", ex.Source(), ex.Line());
        spdlog::error("{}:{}:{}: {}", sourceRange.CodeFile()->FileName(), sourceRange.StartLine() + 1,
                      sourceRange.StartColumn() + 1, ex.Message());
        // spdlog::error(nlohmann::to_string(ExpressionJsonSerializer::ExpressionToJson(ex.Tree())));
    }
    catch (ScopeException &ex)
    {
        spdlog::error("Scope Exception: {}, Name: {}", ex.Message(), UTF32ToUTF8(ex.Name()));
    }
    catch (CompilationException &ex)
    {
        spdlog::error("Compilation Exception: {}", ex.Message());
        spdlog::error("Compiler source code file: {}, line: {}", ex.Source(), ex.Line());
    }
    catch (std::exception &ex)
    {
        spdlog::error("Unexpected error: {}", ex.what());
    }
}

int main(int argc, char **argv)
{
    CLI::App app{"Cygni Compiler"};

    argv = app.ensure_utf8(argv);

    std::string input_file_path;
    std::string output_file_path;
    app.add_option("-i,--input", input_file_path, "Path to the input Cygni source file (.cyg).")->required();
    app.add_option("-o,--output", output_file_path, "Path to the output Flint bytecode file (.fbc).")->required();

    CLI11_PARSE(app, argc, argv);

    TryCompile(input_file_path, output_file_path);
    return 0;
}