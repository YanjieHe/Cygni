# Cygni

Cygni is an experimental, statically typed, expression-oriented language that compiles to [Flint bytecode](https://github.com/YanjieHe/FlintByteCode) and runs on [flint-vm](https://github.com/YanjieHe/flint-vm).

The project is a learning-oriented compiler and language-design project. Its syntax and semantics are still evolving.

## Example

```cygni
module Example {
    func Max(a: Int, b: Int): Int {
        if (a > b) { a; } else { b; }
    }

    func Main(): Int {
        Max(40, 2);
    }
}
```

Cygni currently supports modules and multi-file compilation, primitive types, block and conditional expressions, loops, structures, interfaces with dynamic dispatch, native functions, Unicode strings, and typed array access. General generics, nested lambdas, and source-level array construction and literals are not complete yet.

See the [language specification](docs/LANGUAGE_SPEC.md) for the current syntax. Some design areas are still under discussion, so the implementation remains the final reference when the specification and code differ.

## Build and Test

Requirements:

- CMake
- A C++17 compiler
- Git, used by CMake to fetch FlintByteCode

```bash
cmake -S . -B build
cmake --build build --parallel
./build/tests/cygni-tests
```

FlintByteCode is fetched from its `master` branch by default. To use a local checkout instead:

```bash
cmake -S . -B build \
  -DFETCHCONTENT_SOURCE_DIR_FLINTBYTECODE=/path/to/FlintByteCode
```

## Compile and Run

Compile one or more Cygni source files into a `.fbc` file:

```bash
./build/src/cygni -i example.cyg -o example.fbc
./build/src/cygni -i math.cyg app.cyg -o app.fbc
```

Run the result with a separately built flint-vm checkout:

```bash
/path/to/flint-vm/build/src/flint-vm example.fbc
```

The VM also accepts `--disassemble` to print bytecode instead of executing it.

## Compilation Flow

1. **Read source** – `Main.cpp` reads UTF-8 files, while `SourceCodeFile` identifies each source for diagnostics. Source text is converted to UTF-32 so the compiler can work consistently with Unicode code points.
2. **Tokenize** – `LexicalAnalysis::Lexer` produces tokens and reports malformed input through `LexicalException`.
3. **Parse** – `SyntaxAnalysis::Parser` builds a shared namespace and expression tree for all input files.
4. **Type check** – `Visitors::TypeChecker` validates expressions, declarations, subtyping, and interface implementations.
5. **Assign locations** – `Visitors::NameLocator` assigns global indexes and bytecode locations to functions, variables, structures, interfaces, and native functions.
6. **Emit bytecode** – `Visitors::Compiler` produces functions, metadata, vtables, native-library references, and the entry point in a `flint_bytecode::ByteCodeProgram`.
7. **Write output** – FlintByteCode serializes the program to the requested `.fbc` file.

Compiler errors are reported with source locations where available. Internal bytecode-generation failures use `CompilationException` diagnostics.

## Related Projects

- [FlintByteCode](https://github.com/YanjieHe/FlintByteCode) – bytecode model and serializer
- [flint-vm](https://github.com/YanjieHe/flint-vm) – bytecode loader and virtual machine

## License

See [LICENSE](LICENSE).
