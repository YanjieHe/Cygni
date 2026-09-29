#include <catch2/catch.hpp>

#include "Compilation/CompilationContext.hpp"
#include "LexicalAnalysis/Lexer.hpp"
#include "SyntaxAnalysis/Parser.hpp"
#include "Visitors/Compiler.hpp"
#include "Visitors/NameLocator.hpp"
#include "Visitors/TypeChecker.hpp"

#include "Utility/UTF32Functions.hpp"
#include <bit_converter/bit_converter.hpp>

#include <any>

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
    NameLocator nameLocator = NameLocator(compilationContext.GetNamespaceFactory(), typeChecker);
    nameLocator.InitializeSymbolCounters(&nameInfoScope);
    nameLocator.RegisterAllInfo(&nameInfoScope);
    nameLocator.CheckNamespace(&nameInfoScope);

    Compiler compiler(typeChecker, nameLocator, compilationContext.GetNamespaceFactory());
    std::vector<flint_bytecode::GlobalVariable> globalVariables(nameInfoScope.Get(GLOBAL_VARIABLE_COUNT).Number());
    std::vector<flint_bytecode::Function> functions(nameInfoScope.Get(GLOBAL_FUNCTION_COUNT).Number());
    std::vector<flint_bytecode::NativeFunction> nativeFunctions(
        nameInfoScope.Get(GLOBAL_NATIVE_FUNCTION_COUNT).Number());
    std::vector<flint_bytecode::StructureMeta> structures(nameInfoScope.Get(GLOBAL_STRUCTURE_COUNT).Number());
    std::vector<flint_bytecode::InterfaceMeta> interfaces(nameInfoScope.Get(GLOBAL_INTERFACE_COUNT).Number());
    compiler.CompileNamespace(globalVariables, functions, nativeFunctions, structures, interfaces);
    flint_bytecode::ByteCodeProgram program(globalVariables, structures, functions, {}, nativeFunctions, interfaces,
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
    NameLocator nameLocator = NameLocator(compilationContext.GetNamespaceFactory(), typeChecker);
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
    std::vector<flint_bytecode::InterfaceMeta> interfaces(nameInfoScope.Get(GLOBAL_INTERFACE_COUNT).Number());
    compiler.CompileNamespace(globalVariables, functions, nativeFunctions, structures, interfaces);
    flint_bytecode::ByteCodeProgram program(globalVariables, structures, functions, {}, nativeFunctions, interfaces,
                                            compiler.EntryPoint());

    return program;
}

static const flint_bytecode::Function *FindCompiledFunction(const flint_bytecode::ByteCodeProgram &program,
                                                            const std::string &name)
{
    for (const flint_bytecode::Function &function : program.Functions())
    {
        if (function.Name() == name)
        {
            return &function;
        }
    }

    return nullptr;
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
    // [20-23]: interfaces count
    // [24-27]: entryPoint

    REQUIRE(bytes.size() >= 28); // At least the header

    int32_t globalVarCount = bit_converter::bytes_to_i32(bytes.begin() + 0, true);
    int32_t structCount = bit_converter::bytes_to_i32(bytes.begin() + 4, true);
    int32_t funcCount = bit_converter::bytes_to_i32(bytes.begin() + 8, true);
    int32_t nativeLibCount = bit_converter::bytes_to_i32(bytes.begin() + 12, true);
    int32_t nativeFuncCount = bit_converter::bytes_to_i32(bytes.begin() + 16, true);
    int32_t interfaceCount = bit_converter::bytes_to_i32(bytes.begin() + 20, true);
    int32_t entryPoint = bit_converter::bytes_to_i32(bytes.begin() + 24, true);

    REQUIRE(globalVarCount == 0);         // No global variables
    REQUIRE(structCount == 0);            // No structures
    REQUIRE(funcCount == 2);              // Square and Main
    REQUIRE(nativeLibCount == 0);         // No native libraries
    REQUIRE(nativeFuncCount == 0);        // No native functions
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

    REQUIRE(bytes.size() >= 28);

    int32_t funcCount = bit_converter::bytes_to_i32(bytes.begin() + 8, true);
    int32_t entryPoint = bit_converter::bytes_to_i32(bytes.begin() + 24, true);

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

    REQUIRE(bytes.size() >= 28);

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

    REQUIRE(bytes.size() >= 28);

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

    REQUIRE(bytes.size() >= 28);

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

    REQUIRE(bytes.size() >= 28);

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

    REQUIRE(bytes.size() >= 28);

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

    REQUIRE(bytes.size() >= 28);

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

    REQUIRE(bytes.size() >= 28);

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

    REQUIRE(bytes.size() >= 28);

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

    REQUIRE(bytes.size() >= 28);

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

    REQUIRE(bytes.size() >= 28);

    int32_t funcCount = bit_converter::bytes_to_i32(bytes.begin() + 8, true);
    REQUIRE(funcCount == 3); // Add, Sum3, Main
}

// ============================================================================
// Structure and Method Tests
// ============================================================================

TEST_CASE("standalone function argument slots and locals are compiled separately",
          "[Compiler][Function][FrameLayout]")
{
    flint_bytecode::ByteCodeProgram program = CompileProgram(
        U"module Workshop { "
        U"  func estimate(materials: Int, labor: Int): Int { "
        U"    var total = materials + labor; total; "
        U"  } "
        U"  func Main(): Int { estimate(12, 8); } "
        U"}");

    const flint_bytecode::Function *estimate = nullptr;
    for (const flint_bytecode::Function &function : program.Functions())
    {
        if (function.Name() == "estimate")
        {
            estimate = &function;
            break;
        }
    }

    REQUIRE(estimate != nullptr);
    REQUIRE(estimate->ArgsSize() == 2);
    REQUIRE(estimate->Locals() == 1);
}

TEST_CASE("structure method argument slots include the receiver", "[Compiler][Structure][Method][FrameLayout]")
{
    flint_bytecode::ByteCodeProgram program = CompileProgram(
        U"module Workshop { "
        U"  struct Counter { value: Int; "
        U"    func current(): Int { this.value; } "
        U"    func increase(amount: Int): Int { "
        U"      var updated = this.value + amount; updated; "
        U"    } "
        U"  } "
        U"  func Main(): Int { 0; } "
        U"}");

    const flint_bytecode::Function *current = nullptr;
    const flint_bytecode::Function *increase = nullptr;
    for (const flint_bytecode::Function &function : program.Functions())
    {
        if (function.Name() == "current")
        {
            current = &function;
        }
        else if (function.Name() == "increase")
        {
            increase = &function;
        }
    }

    REQUIRE(current != nullptr);
    REQUIRE(current->ArgsSize() == 1); // receiver
    REQUIRE(current->Locals() == 0);

    REQUIRE(increase != nullptr);
    REQUIRE(increase->ArgsSize() == 2); // receiver + amount
    REQUIRE(increase->Locals() == 1);   // updated
}

TEST_CASE("test struct with method compiles", "[Compiler][Structure][Method]")
{
    flint_bytecode::ByteCodeProgram program = CompileProgram(
        U"module A { struct Counter { value: Int; "
        U"  func get(): Int { this.value; } } "
        U"func Main(): Int { var c = new Counter { value = 42; }; c.get(); } }");

    flint_bytecode::ByteCode byteCode;
    program.Compile(byteCode);
    const std::vector<flint_bytecode::Byte> &bytes = byteCode.GetBytes();

    REQUIRE(bytes.size() >= 28);

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

    REQUIRE(bytes.size() >= 28);

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

    REQUIRE(bytes.size() >= 28);

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

    REQUIRE(bytes.size() >= 28);

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

    REQUIRE(bytes.size() >= 28);

    int32_t globalVarCount = bit_converter::bytes_to_i32(bytes.begin() + 0, true);
    int32_t structCount = bit_converter::bytes_to_i32(bytes.begin() + 4, true);
    int32_t funcCount = bit_converter::bytes_to_i32(bytes.begin() + 8, true);

    REQUIRE(globalVarCount == 0);
    REQUIRE(structCount == 1); // Box
    REQUIRE(funcCount == 3);   // helper + Box::doubled + Main
}

// ============================================================================
// Interface VTable Tests
// ============================================================================

TEST_CASE("structure vtable follows interface method slot order", "[Compiler][Structure][Interface][VTable]")
{
    flint_bytecode::ByteCodeProgram program = CompileProgram(
        U"module Arena { "
        U"  interface Combatant { func attack(): Int; func defend(): Int; } "
        U"  struct Knight <: Combatant { "
        U"    func defend(): Int { 1; } "
        U"    func attack(): Int { 2; } "
        U"  } "
        U"  func Main(): Int { 0; } "
        U"}");

    REQUIRE(program.Structures().size() == 1);
    const flint_bytecode::StructureMeta &knight = program.Structures().at(0);
    const std::vector<flint_bytecode::VTableEntry> &vtableEntries = knight.VTableEntries();

    REQUIRE(knight.Name() == "Arena::Knight");
    REQUIRE(vtableEntries.size() == 1);
    REQUIRE(vtableEntries[0].InterfaceIndex() == 0);

    // Main is function 0. Knight::defend and Knight::attack are functions 1 and 2,
    // but the Combatant slots are [attack, defend].
    REQUIRE(vtableEntries[0].MethodFunctionIndices() == std::vector<int32_t>{2, 1});
}

TEST_CASE("structure vtable includes inherited interfaces once in parent-first order",
          "[Compiler][Structure][Interface][VTable]")
{
    flint_bytecode::ByteCodeProgram program = CompileProgram(
        U"module Bestiary { "
        U"  interface Entity { func id(): Int; } "
        U"  interface Movable <: Entity { func move(): Int; } "
        U"  interface Attackable <: Entity { func attack(): Int; } "
        U"  interface Monster <: Movable, Attackable { func dropLoot(): Int; } "
        U"  struct Dragon <: Monster { "
        U"    func dropLoot(): Int { 1; } "
        U"    func id(): Int { 2; } "
        U"    func attack(): Int { 3; } "
        U"    func move(): Int { 4; } "
        U"  } "
        U"  func Main(): Int { 0; } "
        U"}");

    REQUIRE(program.Structures().size() == 1);
    const flint_bytecode::StructureMeta &dragon = program.Structures().at(0);
    const std::vector<flint_bytecode::VTableEntry> &vtableEntries = dragon.VTableEntries();

    REQUIRE(dragon.Name() == "Bestiary::Dragon");
    REQUIRE(vtableEntries.size() == 4);

    // The shared Entity interface appears once, followed by Movable, Attackable, and Monster.
    REQUIRE(vtableEntries[0].InterfaceIndex() == 0);
    REQUIRE(vtableEntries[1].InterfaceIndex() == 1);
    REQUIRE(vtableEntries[2].InterfaceIndex() == 2);
    REQUIRE(vtableEntries[3].InterfaceIndex() == 3);

    // Main is function 0. Dragon methods are assigned indices in their declaration order:
    // dropLoot=1, id=2, attack=3, move=4.
    REQUIRE(vtableEntries[0].MethodFunctionIndices() == std::vector<int32_t>{2});
    REQUIRE(vtableEntries[1].MethodFunctionIndices() == std::vector<int32_t>{2, 4});
    REQUIRE(vtableEntries[2].MethodFunctionIndices() == std::vector<int32_t>{2, 3});
    REQUIRE(vtableEntries[3].MethodFunctionIndices() == std::vector<int32_t>{2, 4, 3, 1});
}

// ============================================================================
// Interface Method Call Tests
// ============================================================================

TEST_CASE("interface method call emits receiver arguments and method reference",
          "[Compiler][Interface][Method][Call]")
{
    flint_bytecode::ByteCodeProgram program = CompileProgram(
        U"module Sanctuary { "
        U"  interface Healer { func heal(amount: Int): Int; } "
        U"  struct Priest <: Healer { func heal(amount: Int): Int { amount; } } "
        U"  func restore(healer: Healer): Int { healer.heal(1); } "
        U"  func Main(): Int { var priest = new Priest { }; restore(priest); } "
        U"}");

    const flint_bytecode::Function *restore = FindCompiledFunction(program, "restore");
    REQUIRE(restore != nullptr);
    REQUIRE(restore->ConstantPool().size() == 1);

    const flint_bytecode::Constant &constant = restore->ConstantPool().front();
    REQUIRE(constant.GetConstantKind() ==
            flint_bytecode::ConstantKind::CONSTANT_KIND_INTERFACE_METHOD_REFERENCE);
    const flint_bytecode::InterfaceMethodRef &reference =
        std::any_cast<const flint_bytecode::InterfaceMethodRef &>(constant.GetValue());
    REQUIRE(reference.InterfaceIndex() == 0);
    REQUIRE(reference.MethodIndex() == 0);

    const std::vector<flint_bytecode::Byte> &code = restore->Code().GetBytes();
    REQUIRE(code.size() >= 5);
    REQUIRE(code[0] == static_cast<flint_bytecode::Byte>(flint_bytecode::OpCode::PUSH_LOCAL_OBJECT));
    REQUIRE(code[1] == 0); // receiver slot
    REQUIRE(code[2] == static_cast<flint_bytecode::Byte>(flint_bytecode::OpCode::PUSH_I32_1));
    REQUIRE(code[3] == static_cast<flint_bytecode::Byte>(flint_bytecode::OpCode::INVOKE_INTERFACE));
    REQUIRE(code[4] == 0); // interface method reference constant
}

TEST_CASE("interface method call uses flattened method slot order", "[Compiler][Interface][Method][Call]")
{
    flint_bytecode::ByteCodeProgram program = CompileProgram(
        U"module Arena { "
        U"  interface Combatant { func attack(): Int; func defend(): Int; } "
        U"  func guard(combatant: Combatant): Int { combatant.defend(); } "
        U"  func Main(): Int { 0; } "
        U"}");

    const flint_bytecode::Function *guard = FindCompiledFunction(program, "guard");
    REQUIRE(guard != nullptr);
    REQUIRE(guard->ConstantPool().size() == 1);

    const flint_bytecode::Constant &constant = guard->ConstantPool().front();
    REQUIRE(constant.GetConstantKind() ==
            flint_bytecode::ConstantKind::CONSTANT_KIND_INTERFACE_METHOD_REFERENCE);
    const flint_bytecode::InterfaceMethodRef &reference =
        std::any_cast<const flint_bytecode::InterfaceMethodRef &>(constant.GetValue());
    REQUIRE(reference.InterfaceIndex() == 0);
    REQUIRE(reference.MethodIndex() == 1); // defend follows attack
}

TEST_CASE("inherited interface method call uses derived interface flattened slot",
          "[Compiler][Interface][Inheritance][Method][Call]")
{
    flint_bytecode::ByteCodeProgram program = CompileProgram(
        U"module World { "
        U"  interface Entity { func id(): Int; } "
        U"  interface Character <: Entity { func level(): Int; } "
        U"  func inspect(character: Character): Int { character.id(); } "
        U"  func Main(): Int { 0; } "
        U"}");

    const flint_bytecode::Function *inspect = FindCompiledFunction(program, "inspect");
    REQUIRE(inspect != nullptr);
    REQUIRE(inspect->ConstantPool().size() == 1);

    const flint_bytecode::Constant &constant = inspect->ConstantPool().front();
    REQUIRE(constant.GetConstantKind() ==
            flint_bytecode::ConstantKind::CONSTANT_KIND_INTERFACE_METHOD_REFERENCE);
    const flint_bytecode::InterfaceMethodRef &reference =
        std::any_cast<const flint_bytecode::InterfaceMethodRef &>(constant.GetValue());
    REQUIRE(reference.InterfaceIndex() == 1); // Character
    REQUIRE(reference.MethodIndex() == 0);    // inherited Entity::id
}
