# Cygni

Cygni is a small compiler that reads Cygni source files and emits Flint bytecode. The focus of this document is simply to describe how code flows through the toolchain.

## Compilation Flow

1. **Read source** – `SourceCodeFile` loads UTF-8 text and `UTF8ToUTF32` converts it so Unicode-aware components can index characters safely.
2. **Tokenize** – `LexicalAnalysis::Lexer` produces a `std::vector<Token>` while raising `LexicalException` on malformed input.
3. **Parse** – `SyntaxAnalysis::Parser` consumes the tokens, builds namespaces/expressions, and throws `ParserException` with precise `SourceRange` info when grammar violations occur.
4. **Type check** – `Visitors::TypeChecker` walks the parsed namespace, verifies expressions against declared `Expressions::Type` definitions, and records errors through `TreeException` when invariants fail.
5. **Resolve names** – `Visitors::NameLocator` fills symbol tables, tracks counts for globals/functions/structures/native calls, and surfaces `ScopeException` if symbols clash.
6. **Emit bytecode** – `Visitors::Compiler` maps the validated AST into `flint_bytecode::ByteCodeProgram`, populates globals/functions/native libraries, and serializes the final `ByteCode` to disk.
7. **CLI wrapper** – `Main.cpp` wires the steps together, using CLI11 for arguments (`-i/--input`, `-o/--output`) and spdlog for logging.

If any stage fails, the corresponding exception (`LexicalException`, `ParserException`, `TreeException`, or `ScopeException`) provides file, line, and column details so the user can fix the input source.

## Dependencies

Cygni depends on [FlintByteCode](https://github.com/YanjieHe/FlintByteCode) for bytecode generation. It is fetched automatically via CMake's `FetchContent` during configuration.

For local development, you can point CMake to a local copy of FlintByteCode to avoid fetching from GitHub and to test local changes immediately:

```bash
cmake -B build -DFETCHCONTENT_SOURCE_DIR_FLINTBYTECODE=/path/to/FlintByteCode
```

If the variable is not set, CMake will clone the `master` branch from GitHub automatically.

## Build and Run Tests

```bash
mkdir -p build
cd build
cmake ..
cmake --build . --parallel
./tests/cygni-tests
```

If the build is successful, all test cases should pass.

## License

See LICENSE for details.