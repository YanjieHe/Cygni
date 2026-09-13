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
using namespace Cygni::Compilation;

static flint_bytecode::ByteCodeProgram CompileProgram(const std::u32string &sourceCode)
{
    std::shared_ptr<SourceCodeFile> sourceCodeFile = std::make_shared<SourceCodeFile>("source-code-file");
    Lexer lexer(sourceCodeFile, sourceCode);
    std::vector<Token> tokens = lexer.ReadAll();
    CompilationContext compilationContext;

    Parser parser(tokens, sourceCodeFile, compilationContext);
    parser.ParseNamespace();

    TypeChecker typeChecker(compilationContext.GetNamespaceFactory(), compilationContext.GetExpressionFactory());

    Scope<const Type *> scope;
    typeChecker.CheckNamespace(&scope);

    Scope<NameInfo> nameInfoScope;
    NameLocator nameLocator = NameLocator(compilationContext.GetNamespaceFactory());
    nameLocator.InitializeSymbolCounters(&nameInfoScope);
    nameLocator.RegisterAllInfo(&nameInfoScope);
    nameLocator.CheckNamespace(&nameInfoScope);

    Compiler compiler(typeChecker, nameLocator, compilationContext.GetNamespaceFactory());
    std::vector<flint_bytecode::GlobalVariable> globalVariables(nameInfoScope.Get(GLOBAL_VARIABLE_COUNT).Number());
    std::vector<flint_bytecode::Function> functions(nameInfoScope.Get(GLOBAL_FUNCTION_COUNT).Number());
    std::vector<flint_bytecode::NativeFunction> nativeFunctions(
        nameInfoScope.Get(GLOBAL_NATIVE_FUNCTION_COUNT).Number());
    std::vector<flint_bytecode::StructureMeta> structures(nameInfoScope.Get(GLOBAL_STRUCTURE_COUNT).Number());
    compiler.CompileNamespace(globalVariables, functions, nativeFunctions, structures);
    flint_bytecode::ByteCodeProgram program(globalVariables, structures, functions, {}, nativeFunctions, {}, {},
                                            compiler.EntryPoint());

    return program;
}

static flint_bytecode::ByteCodeProgram CompileMultipleFiles(const std::vector<std::u32string> &sourceFiles)
{
    CompilationContext compilationContext;

    // Phase 1: Parse all files, all namespaces go into the same root
    for (size_t i = 0; i < sourceFiles.size(); i++)
    {
        std::string fileName = "source-file-" + std::to_string(i) + ".cyg";
        std::shared_ptr<SourceCodeFile> sourceCodeFile = std::make_shared<SourceCodeFile>(fileName);
        Lexer lexer(sourceCodeFile, sourceFiles[i]);
        std::vector<Token> tokens = lexer.ReadAll();

        Parser parser(tokens, sourceCodeFile, compilationContext);
        parser.ParseNamespace();
    }

    // Phase 2: Type check the entire namespace tree
    TypeChecker typeChecker(compilationContext.GetNamespaceFactory(), compilationContext.GetExpressionFactory());
    Scope<const Type *> scope;
    typeChecker.CheckNamespace(&scope);

    // Phase 3: Locate names
    Scope<NameInfo> nameInfoScope;
    NameLocator nameLocator = NameLocator(compilationContext.GetNamespaceFactory());
    nameLocator.InitializeSymbolCounters(&nameInfoScope);
    nameLocator.RegisterAllInfo(&nameInfoScope);
    nameLocator.CheckNamespace(&nameInfoScope);

    // Phase 4: Compile
    Compiler compiler(typeChecker, nameLocator, compilationContext.GetNamespaceFactory());
    std::vector<flint_bytecode::GlobalVariable> globalVariables(nameInfoScope.Get(GLOBAL_VARIABLE_COUNT).Number());
    std::vector<flint_bytecode::Function> functions(nameInfoScope.Get(GLOBAL_FUNCTION_COUNT).Number());
    std::vector<flint_bytecode::NativeFunction> nativeFunctions(
        nameInfoScope.Get(GLOBAL_NATIVE_FUNCTION_COUNT).Number());
    std::vector<flint_bytecode::StructureMeta> structures(nameInfoScope.Get(GLOBAL_STRUCTURE_COUNT).Number());
    compiler.CompileNamespace(globalVariables, functions, nativeFunctions, structures);
    flint_bytecode::ByteCodeProgram program(globalVariables, structures, functions, {}, nativeFunctions, {}, {},
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
    // [20-23]: interfaceMethodReferences count
    // [24-27]: interfaces count
    // [28-31]: entryPoint

    REQUIRE(bytes.size() >= 32); // At least the header

    int32_t globalVarCount = bit_converter::bytes_to_i32(bytes.begin() + 0, true);
    int32_t structCount = bit_converter::bytes_to_i32(bytes.begin() + 4, true);
    int32_t funcCount = bit_converter::bytes_to_i32(bytes.begin() + 8, true);
    int32_t nativeLibCount = bit_converter::bytes_to_i32(bytes.begin() + 12, true);
    int32_t nativeFuncCount = bit_converter::bytes_to_i32(bytes.begin() + 16, true);
    int32_t interfaceMethodRefCount = bit_converter::bytes_to_i32(bytes.begin() + 20, true);
    int32_t interfaceCount = bit_converter::bytes_to_i32(bytes.begin() + 24, true);
    int32_t entryPoint = bit_converter::bytes_to_i32(bytes.begin() + 28, true);

    REQUIRE(globalVarCount == 0);         // No global variables
    REQUIRE(structCount == 0);            // No structures
    REQUIRE(funcCount == 2);              // Square and Main
    REQUIRE(nativeLibCount == 0);         // No native libraries
    REQUIRE(nativeFuncCount == 0);        // No native functions
    REQUIRE(interfaceMethodRefCount == 0); // No interface method refs
    REQUIRE(interfaceCount == 0);         // No interfaces
    REQUIRE(entryPoint == 1);             // Main is the second function (index 1)
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

    REQUIRE(bytes.size() >= 32);

    int32_t funcCount = bit_converter::bytes_to_i32(bytes.begin() + 8, true);
    int32_t entryPoint = bit_converter::bytes_to_i32(bytes.begin() + 28, true);

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

    REQUIRE(bytes.size() >= 32);

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

    REQUIRE(bytes.size() >= 32);

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

    REQUIRE(bytes.size() >= 32);

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

    REQUIRE(bytes.size() >= 32);

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

    REQUIRE(bytes.size() >= 32);

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

    REQUIRE(bytes.size() >= 32);

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

    REQUIRE(bytes.size() >= 32);

    int32_t globalVarCount = bit_converter::bytes_to_i32(bytes.begin() + 0, true);
    REQUIRE(globalVarCount == 1);
}

// ============================================================================
// Multi-File Compilation Tests
// ============================================================================

TEST_CASE("test multi-file compilation with cross-module call", "[Compiler][MultiFile]")
{
    // File 1: module A with nested module Math
    std::u32string file1 = U"module A { module Math { "
                           U"  func Square(x: Int): Int { x * x; } "
                           U"  func Double(x: Int): Int { x + x; } "
                           U"} }";

    // File 2: module B that uses A::Math functions
    std::u32string file2 = U"module B { "
                           U"  func Compute(x: Int): Int { A::Math::Square(x) + A::Math::Double(x); } "
                           U"  func Main(): Int { Compute(5); } "
                           U"}";

    flint_bytecode::ByteCodeProgram program = CompileMultipleFiles({file1, file2});

    flint_bytecode::ByteCode byteCode;
    program.Compile(byteCode);
    const std::vector<flint_bytecode::Byte> &bytes = byteCode.GetBytes();

    REQUIRE(bytes.size() >= 32);

    int32_t globalVarCount = bit_converter::bytes_to_i32(bytes.begin() + 0, true);
    int32_t structCount = bit_converter::bytes_to_i32(bytes.begin() + 4, true);
    int32_t funcCount = bit_converter::bytes_to_i32(bytes.begin() + 8, true);

    REQUIRE(globalVarCount == 0);
    REQUIRE(structCount == 0);
    REQUIRE(funcCount == 4); // Square, Double, Compute, Main
}

TEST_CASE("test multi-file compilation with shared global variable", "[Compiler][MultiFile]")
{
    // File 1: module Config with a global variable
    std::u32string file1 = U"module Config { "
                           U"  var multiplier: Int = 10; "
                           U"}";

    // File 2: module App that uses Config's global variable
    std::u32string file2 = U"module App { "
                           U"  func Apply(x: Int): Int { x * Config::multiplier; } "
                           U"  func Main(): Int { Apply(5); } "
                           U"}";

    flint_bytecode::ByteCodeProgram program = CompileMultipleFiles({file1, file2});

    flint_bytecode::ByteCode byteCode;
    program.Compile(byteCode);
    const std::vector<flint_bytecode::Byte> &bytes = byteCode.GetBytes();

    REQUIRE(bytes.size() >= 32);

    int32_t globalVarCount = bit_converter::bytes_to_i32(bytes.begin() + 0, true);
    int32_t funcCount = bit_converter::bytes_to_i32(bytes.begin() + 8, true);

    REQUIRE(globalVarCount == 1); // Config::multiplier
    REQUIRE(funcCount == 3);      // multiplier#Initializer, Apply, Main
}

TEST_CASE("test multi-file compilation with three files", "[Compiler][MultiFile]")
{
    // File 1: module Utils
    std::u32string file1 = U"module Utils { "
                           U"  func Add(a: Int, b: Int): Int { a + b; } "
                           U"}";

    // File 2: module Math (uses Utils)
    std::u32string file2 = U"module Math { "
                           U"  func Sum3(a: Int, b: Int, c: Int): Int { Utils::Add(Utils::Add(a, b), c); } "
                           U"}";

    // File 3: module Main (uses Math)
    std::u32string file3 = U"module Main { "
                           U"  func Main(): Int { Math::Sum3(1, 2, 3); } "
                           U"}";

    flint_bytecode::ByteCodeProgram program = CompileMultipleFiles({file1, file2, file3});

    flint_bytecode::ByteCode byteCode;
    program.Compile(byteCode);
    const std::vector<flint_bytecode::Byte> &bytes = byteCode.GetBytes();

    REQUIRE(bytes.size() >= 32);

    int32_t funcCount = bit_converter::bytes_to_i32(bytes.begin() + 8, true);
    REQUIRE(funcCount == 3); // Add, Sum3, Main
}

// ============================================================================
// Structure and Method Tests
// ============================================================================

TEST_CASE("test struct with method compiles", "[Compiler][Structure][Method]")
{
    flint_bytecode::ByteCodeProgram program = CompileProgram(
        U"module A { struct Counter { value: Int; "
        U"  func get(): Int { this.value; } } "
        U"func Main(): Int { var c = new Counter { value = 42; }; c.get(); } }");

    flint_bytecode::ByteCode byteCode;
    program.Compile(byteCode);
    const std::vector<flint_bytecode::Byte> &bytes = byteCode.GetBytes();

    REQUIRE(bytes.size() >= 32);

    int32_t structCount = bit_converter::bytes_to_i32(bytes.begin() + 4, true);
    int32_t funcCount = bit_converter::bytes_to_i32(bytes.begin() + 8, true);

    REQUIRE(structCount == 1); // Counter
    REQUIRE(funcCount == 2);   // Counter::get + Main
}

TEST_CASE("test struct with multiple methods compiles", "[Compiler][Structure][Method]")
{
    flint_bytecode::ByteCodeProgram program = CompileProgram(
        U"module A { struct Vec2 { x: Int; y: Int; "
        U"  func getX(): Int { this.x; } "
        U"  func getY(): Int { this.y; } "
        U"  func sum(): Int { this.x + this.y; } } "
        U"func Main(): Int { var v = new Vec2 { x = 3; y = 4; }; v.sum(); } }");

    flint_bytecode::ByteCode byteCode;
    program.Compile(byteCode);
    const std::vector<flint_bytecode::Byte> &bytes = byteCode.GetBytes();

    REQUIRE(bytes.size() >= 32);

    int32_t structCount = bit_converter::bytes_to_i32(bytes.begin() + 4, true);
    int32_t funcCount = bit_converter::bytes_to_i32(bytes.begin() + 8, true);

    REQUIRE(structCount == 1); // Vec2
    REQUIRE(funcCount == 4);   // getX + getY + sum + Main
}

TEST_CASE("test struct method with params compiles", "[Compiler][Structure][Method]")
{
    flint_bytecode::ByteCodeProgram program = CompileProgram(
        U"module A { struct Calc { value: Int; "
        U"  func add(x: Int): Int { this.value + x; } } "
        U"func Main(): Int { var c = new Calc { value = 10; }; c.add(5); } }");

    flint_bytecode::ByteCode byteCode;
    program.Compile(byteCode);
    const std::vector<flint_bytecode::Byte> &bytes = byteCode.GetBytes();

    REQUIRE(bytes.size() >= 32);

    int32_t structCount = bit_converter::bytes_to_i32(bytes.begin() + 4, true);
    int32_t funcCount = bit_converter::bytes_to_i32(bytes.begin() + 8, true);

    REQUIRE(structCount == 1); // Calc
    REQUIRE(funcCount == 2);   // Calc::add + Main
}

TEST_CASE("test multiple structs compile", "[Compiler][Structure][Method]")
{
    flint_bytecode::ByteCodeProgram program = CompileProgram(
        U"module A { "
        U"  struct Point { x: Int; y: Int; } "
        U"  struct Rect { width: Int; height: Int; "
        U"    func area(): Int { this.width * this.height; } } "
        U"  func Main(): Int { var r = new Rect { width = 3; height = 4; }; r.area(); } }");

    flint_bytecode::ByteCode byteCode;
    program.Compile(byteCode);
    const std::vector<flint_bytecode::Byte> &bytes = byteCode.GetBytes();

    REQUIRE(bytes.size() >= 32);

    int32_t structCount = bit_converter::bytes_to_i32(bytes.begin() + 4, true);
    int32_t funcCount = bit_converter::bytes_to_i32(bytes.begin() + 8, true);

    REQUIRE(structCount == 2); // Point + Rect
    REQUIRE(funcCount == 2);   // Rect::area + Main
}

TEST_CASE("test struct with standalone functions compile", "[Compiler][Structure][Method]")
{
    flint_bytecode::ByteCodeProgram program = CompileProgram(
        U"module A { "
        U"  func helper(n: Int): Int { n * 2; } "
        U"  struct Box { value: Int; "
        U"    func doubled(): Int { helper(this.value); } } "
        U"  func Main(): Int { var b = new Box { value = 5; }; b.doubled(); } }");

    flint_bytecode::ByteCode byteCode;
    program.Compile(byteCode);
    const std::vector<flint_bytecode::Byte> &bytes = byteCode.GetBytes();

    REQUIRE(bytes.size() >= 32);

    int32_t globalVarCount = bit_converter::bytes_to_i32(bytes.begin() + 0, true);
    int32_t structCount = bit_converter::bytes_to_i32(bytes.begin() + 4, true);
    int32_t funcCount = bit_converter::bytes_to_i32(bytes.begin() + 8, true);

    REQUIRE(globalVarCount == 0);
    REQUIRE(structCount == 1); // Box
    REQUIRE(funcCount == 3);   // helper + Box::doubled + Main
}