#include <catch2/catch.hpp>

#include "Compilation/CompilationContext.hpp"
#include "LexicalAnalysis/Lexer.hpp"
#include "SyntaxAnalysis/Parser.hpp"
#include "Visitors/Compiler.hpp"
#include "Visitors/NameLocator.hpp"
#include "Visitors/TypeChecker.hpp"

#include "Utility/UTF32Functions.hpp"
#include <bit_converter/bit_converter.hpp>

using namespace Cygni::LexicalAnalysis;
using namespace Cygni::SyntaxAnalysis;
using namespace Cygni::Expressions;
using namespace Cygni::Visitors;

static flint_bytecode::ByteCodeProgram CompileProgram(const std::u32string &sourceCode)
{
    std::shared_ptr<SourceCodeFile> sourceCodeFile = std::make_shared<SourceCodeFile>("source-code-file");
    Lexer lexer(sourceCodeFile, sourceCode);
    std::vector<Token> tokens = lexer.ReadAll();
    Cygni::Compilation::CompilationContext compilationContext;

    Parser parser(tokens, sourceCodeFile, compilationContext);
    parser.ParseNamespace();

    TypeChecker typeChecker(parser.GetNamespaceFactory(), parser.GetExpressionFactory());

    Scope<const Type *> scope;
    typeChecker.CheckNamespace(&scope);

    Scope<NameInfo> nameInfoScope;
    NameLocator nameLocator = NameLocator(parser.GetNamespaceFactory());
    nameLocator.InitializeSymbolCounters(&nameInfoScope);
    nameLocator.RegisterAllInfo(&nameInfoScope);
    nameLocator.CheckNamespace(&nameInfoScope);

    // REQUIRE(nameInfoScope.Get(GLOBAL_FUNCTION_COUNT).Number() == 2);

    Compiler compiler(typeChecker, nameLocator, parser.GetNamespaceFactory());
    std::vector<flint_bytecode::GlobalVariable> globalVariables(nameInfoScope.Get(GLOBAL_VARIABLE_COUNT).Number());
    std::vector<flint_bytecode::Function> functions(nameInfoScope.Get(GLOBAL_FUNCTION_COUNT).Number());
    std::vector<flint_bytecode::NativeFunction> nativeFunctions(
        nameInfoScope.Get(GLOBAL_NATIVE_FUNCTION_COUNT).Number());
    std::vector<flint_bytecode::StructureMeta> structures(nameInfoScope.Get(GLOBAL_STRUCTURE_COUNT).Number());
    compiler.CompileNamespace(globalVariables, functions, nativeFunctions);
    flint_bytecode::ByteCodeProgram program(globalVariables, structures, functions, {}, nativeFunctions,
                                            compiler.EntryPoint());

    return program;
}

TEST_CASE("test compiler", "[Compiler]")
{
    flint_bytecode::ByteCodeProgram program = CompileProgram(U"module A { func Square(x: Int): Int { x * x; } "
                                                             U"func Main(): Int { Square(3); } }");

    // Compile to bytecode and verify the header
    flint_bytecode::ByteCode byteCode;
    program.Compile(byteCode);
    const std::vector<flint_bytecode::Byte> &bytes = byteCode.GetBytes();

    // ByteCodeProgram header format (little-endian int32):
    // [0-3]:   globalVariables count
    // [4-7]:   structures count
    // [8-11]:  functions count
    // [12-15]: nativeLibraries count
    // [16-19]: nativeFunctions count
    // [20-23]: entryPoint

    REQUIRE(bytes.size() >= 24); // At least the header

    int32_t globalVarCount = bit_converter::bytes_to_i32(bytes.begin() + 0, true);
    int32_t structCount = bit_converter::bytes_to_i32(bytes.begin() + 4, true);
    int32_t funcCount = bit_converter::bytes_to_i32(bytes.begin() + 8, true);
    int32_t nativeLibCount = bit_converter::bytes_to_i32(bytes.begin() + 12, true);
    int32_t nativeFuncCount = bit_converter::bytes_to_i32(bytes.begin() + 16, true);
    int32_t entryPoint = bit_converter::bytes_to_i32(bytes.begin() + 20, true);

    REQUIRE(globalVarCount == 0);  // No global variables
    REQUIRE(structCount == 0);     // No structures
    REQUIRE(funcCount == 2);       // Square and Main
    REQUIRE(nativeLibCount == 0);  // No native libraries
    REQUIRE(nativeFuncCount == 0); // No native functions
    REQUIRE(entryPoint == 1);      // Main is the second function (index 1)
}

// ============================================================================
// Control Flow Tests - Conditional
// ============================================================================

TEST_CASE("test conditional expression compilation", "[Compiler][Conditional]")
{
    flint_bytecode::ByteCodeProgram program =
        CompileProgram(U"module A { func Max(a: Int, b: Int): Int { if (a > b) { a; } else { b; } } "
                       U"func Main(): Int { Max(10, 20); } }");

    flint_bytecode::ByteCode byteCode;
    program.Compile(byteCode);
    const std::vector<flint_bytecode::Byte> &bytes = byteCode.GetBytes();

    REQUIRE(bytes.size() >= 24);

    int32_t funcCount = bit_converter::bytes_to_i32(bytes.begin() + 8, true);
    int32_t entryPoint = bit_converter::bytes_to_i32(bytes.begin() + 20, true);

    REQUIRE(funcCount == 2);
    REQUIRE(entryPoint == 1);
}

TEST_CASE("test nested conditional compilation", "[Compiler][Conditional]")
{
    flint_bytecode::ByteCodeProgram program =
        CompileProgram(U"module A { func Clamp(x: Int, low: Int, high: Int): Int { "
                       U"  if (x < low) { low; } else { if (x > high) { high; } else { x; } } } "
                       U"func Main(): Int { Clamp(5, 0, 10); } }");

    flint_bytecode::ByteCode byteCode;
    program.Compile(byteCode);
    const std::vector<flint_bytecode::Byte> &bytes = byteCode.GetBytes();

    REQUIRE(bytes.size() >= 24);

    int32_t funcCount = bit_converter::bytes_to_i32(bytes.begin() + 8, true);
    REQUIRE(funcCount == 2);
}

// ============================================================================
// Control Flow Tests - While Loop
// ============================================================================

TEST_CASE("test while loop compilation", "[Compiler][WhileLoop]")
{
    flint_bytecode::ByteCodeProgram program =
        CompileProgram(U"module A { func Sum(n: Int): Int { var result = 0; var i = 1; "
                       U"  while (i <= n) { result = result + i; i = i + 1; } result; } "
                       U"func Main(): Int { Sum(10); } }");

    flint_bytecode::ByteCode byteCode;
    program.Compile(byteCode);
    const std::vector<flint_bytecode::Byte> &bytes = byteCode.GetBytes();

    REQUIRE(bytes.size() >= 24);

    int32_t funcCount = bit_converter::bytes_to_i32(bytes.begin() + 8, true);
    REQUIRE(funcCount == 2);
}

TEST_CASE("test nested while loop compilation", "[Compiler][WhileLoop]")
{
    flint_bytecode::ByteCodeProgram program =
        CompileProgram(U"module A { func Multiply(a: Int, b: Int): Int { var result = 0; var i = 0; "
                       U"  while (i < b) { result = result + a; i = i + 1; } result; } "
                       U"func Main(): Int { Multiply(3, 4); } }");

    flint_bytecode::ByteCode byteCode;
    program.Compile(byteCode);
    const std::vector<flint_bytecode::Byte> &bytes = byteCode.GetBytes();

    REQUIRE(bytes.size() >= 24);

    int32_t funcCount = bit_converter::bytes_to_i32(bytes.begin() + 8, true);
    REQUIRE(funcCount == 2);
}

// ============================================================================
// Variable Tests - Local Variables
// ============================================================================

TEST_CASE("test local variable declaration and usage", "[Compiler][Variable]")
{
    flint_bytecode::ByteCodeProgram program =
        CompileProgram(U"module A { func Compute(x: Int): Int { var a = x + 1; var b = a * 2; b; } "
                       U"func Main(): Int { Compute(5); } }");

    flint_bytecode::ByteCode byteCode;
    program.Compile(byteCode);
    const std::vector<flint_bytecode::Byte> &bytes = byteCode.GetBytes();

    REQUIRE(bytes.size() >= 24);

    int32_t funcCount = bit_converter::bytes_to_i32(bytes.begin() + 8, true);
    REQUIRE(funcCount == 2);
}

TEST_CASE("test variable reassignment", "[Compiler][Variable]")
{
    flint_bytecode::ByteCodeProgram program =
        CompileProgram(U"module A { func Increment(x: Int): Int { var a = x; a = a + 1; a = a + 1; a; } "
                       U"func Main(): Int { Increment(0); } }");

    flint_bytecode::ByteCode byteCode;
    program.Compile(byteCode);
    const std::vector<flint_bytecode::Byte> &bytes = byteCode.GetBytes();

    REQUIRE(bytes.size() >= 24);

    int32_t funcCount = bit_converter::bytes_to_i32(bytes.begin() + 8, true);
    REQUIRE(funcCount == 2);
}

// ============================================================================
// Variable Tests - Global Variables
// ============================================================================

TEST_CASE("test global variable compilation", "[Compiler][Variable][Global]")
{
    flint_bytecode::ByteCodeProgram program = CompileProgram(U"module A { var counter: Int = 0; "
                                                             U"func GetCounter(): Int { counter; } "
                                                             U"func Main(): Int { GetCounter(); } }");

    flint_bytecode::ByteCode byteCode;
    program.Compile(byteCode);
    const std::vector<flint_bytecode::Byte> &bytes = byteCode.GetBytes();

    REQUIRE(bytes.size() >= 24);

    int32_t globalVarCount = bit_converter::bytes_to_i32(bytes.begin() + 0, true);
    int32_t funcCount = bit_converter::bytes_to_i32(bytes.begin() + 8, true);

    REQUIRE(globalVarCount == 1);
    REQUIRE(funcCount == 3); // GetCounter, Main, and counter#Initializer
}

TEST_CASE("test global variable assignment", "[Compiler][Variable][Global]")
{
    flint_bytecode::ByteCodeProgram program = CompileProgram(U"module A { var value: Int = 10; "
                                                             U"func SetValue(x: Int): Int { value = x; value; } "
                                                             U"func Main(): Int { SetValue(42); } }");

    flint_bytecode::ByteCode byteCode;
    program.Compile(byteCode);
    const std::vector<flint_bytecode::Byte> &bytes = byteCode.GetBytes();

    REQUIRE(bytes.size() >= 24);

    int32_t globalVarCount = bit_converter::bytes_to_i32(bytes.begin() + 0, true);
    REQUIRE(globalVarCount == 1);
}