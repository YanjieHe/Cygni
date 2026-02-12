# Cygni Language Specification (v0.1)

Cygni is a statically-typed, expression-oriented programming language that compiles to Flint bytecode.

## 1. Lexical Elements

### 1.1 Keywords

```
module  func  var  struct  if  else  while  new  true  false
```

### 1.2 Operators

| Category | Operators |
|----------|-----------|
| Arithmetic | `+`  `-`  `*`  `/`  `%` |
| Comparison | `==`  `!=`  `>`  `>=`  `<`  `<=` |
| Logical | `&&`  `\|\|`  `!` |
| Assignment | `=` |
| Scope Resolution | `::` |
| Member Access | `.` |

### 1.3 Delimiters

```
(  )  {  }  [  ]  ,  ;  :  @
```

### 1.4 Literals

| Type | Examples |
|------|----------|
| Integer | `123`, `-42`, `0` |
| Float | `3.14`, `-0.5`, `1.0` |
| Character | `'a'`, `'Z'`, `'\n'` |
| String | `"hello"`, `"world"` |
| Boolean | `true`, `false` |

### 1.5 Comments

Single-line comments start with `//` and continue to the end of the line:

```cygni
// This is a comment
var x: Int = 42;  // This is also a comment
```

*Note: Block comments (`/* ... */`) are not yet supported.*

---

## 2. Type System

### 2.1 Primitive Types

| Type | Description |
|------|-------------|
| `Int` | 32-bit signed integer |
| `Long` | 64-bit signed integer |
| `Float` | 32-bit floating point |
| `Double` | 64-bit floating point |
| `Bool` | Boolean (`true` or `false`) |
| `Char` | Unicode character |
| `String` | Unicode string |
| `Void` | Unit type (no value) |

### 2.2 User-Defined Types

- **Structures**: Composite types with named fields

### 2.3 Type Syntax

```ebnf
Type          ::= QualifiedName TypeArgs?
TypeArgs      ::= '[' Type (',' Type)* ']'
QualifiedName ::= Identifier ('::' Identifier)*
```

---

## 3. Program Structure

### 3.1 Modules

Modules are the top-level organizational unit. Modules can be nested.

```cygni
module OuterModule {
    module InnerModule {
        // declarations
    }
}
```

A module can contain:
- Nested modules
- Global variables
- Functions
- Structures

### 3.2 Global Variables

Global variables must have explicit type annotations.

```cygni
var name: Type = initializer;
```

Example:
```cygni
var counter: Int = 0;
var message: String = "Hello";
```

### 3.3 Functions

```cygni
func Name(param1: Type1, param2: Type2): ReturnType {
    // function body
}
```

- The last expression in a function body is the return value.
- Functions without a body are external (native) function declarations.

Example:
```cygni
func Add(a: Int, b: Int): Int {
    a + b;
}
```

### 3.4 Structures

```cygni
struct Name {
    field1: Type1;
    field2: Type2;
}
```

Example:
```cygni
struct Point {
    x: Int;
    y: Int;
}
```

---

## 4. Expressions

### 4.1 Operator Precedence (lowest to highest)

| Precedence | Operators | Associativity |
|------------|-----------|---------------|
| 1 | `=` | Right |
| 2 | `\|\|` | Left |
| 3 | `&&` | Left |
| 4 | `!` | Right (unary) |
| 5 | `==` `!=` | Left |
| 6 | `>` `>=` `<` `<=` | Left |
| 7 | `+` `-` | Left |
| 8 | `*` `/` | Left |
| 9 | `+` `-` `!` (unary) | Right |
| 10 | `()` `[]` `.` `::` | Left (postfix) |

### 4.2 Literal Expressions

```cygni
42              // Int
3.14            // Float (Double)
'c'             // Char
"hello"         // String
true            // Bool
false           // Bool
```

### 4.3 Variable Reference

```cygni
x               // local or parameter
counter         // could be global if in scope
```

### 4.4 Qualified Names

Use `::` to reference symbols in other modules:

```cygni
ModuleA::ModuleB::functionName(args)
ModuleA::globalVariable
```

### 4.5 Function Calls

```cygni
functionName(arg1, arg2, arg3)
Math::Square(5)
```

### 4.6 Member Access

```cygni
object.fieldName
point.x
```

### 4.7 Object Creation

```cygni
new TypeName {
    field1 = expr1;
    field2 = expr2;
}
```

Example:
```cygni
var p = new Point { x = 10; y = 20; };
```

### 4.8 Block Expressions

A block is a sequence of statements enclosed in braces. The value of a block is the value of its last expression.

```cygni
{
    var a = 1;
    var b = 2;
    a + b;      // block evaluates to 3
}
```

---

## 5. Statements

### 5.1 Variable Declaration

```cygni
var name = expr;              // type inferred
var name: Type = expr;        // explicit type
```

### 5.2 Expression Statement

Any expression followed by a semicolon:

```cygni
functionCall();
x = x + 1;
```

### 5.3 Conditional Statement

```cygni
if (condition) {
    // then branch
}

if (condition) {
    // then branch
} else {
    // else branch
}

if (condition1) {
    // branch 1
} else if (condition2) {
    // branch 2
} else {
    // else branch
}
```

- Conditional expressions return a value (the value of the executed branch).

### 5.4 While Loop

```cygni
while (condition) {
    // loop body
}
```

---

## 6. Annotations

Annotations provide metadata for declarations.

```cygni
@AnnotationName(arg1 = "value1", arg2 = "value2")
func ...
```

### 6.1 Built-in Annotations

#### `@NativeFunction`

Binds a function declaration to an external C function.

```cygni
@NativeFunction(library = "path/to/library.so", function = "c_function_name")
func ExternalFunc(x: Int): Int;
```

---

## 7. Modules and Visibility

### 7.1 Module Definition

Modules must be defined using nested syntax:

```cygni
// Correct: nested modules
module A {
    module B {
        func Foo(): Int { 42; }
    }
}

// Incorrect: path-style module names are NOT supported
// module A::B { ... }  // This will NOT work
```

### 7.2 Cross-Module References

Use the `::` operator with fully qualified names:

```cygni
module Math {
    func Square(x: Int): Int { x * x; }
}

module Main {
    func Main(): Int {
        Math::Square(5);    // Call function from Math module
    }
}
```

### 7.3 Multi-File Compilation

Multiple source files can be compiled together. All modules are merged into a single namespace tree:

```bash
cygni -i math.cyg utils.cyg main.cyg -o output.fbc
```

---

## 8. Complete Example

```cygni
module Math {
    func Square(x: Int): Int {
        x * x;
    }
    
    func Max(a: Int, b: Int): Int {
        if (a > b) { a; } else { b; }
    }
    
    func Abs(x: Int): Int {
        if (x < 0) { -x; } else { x; }
    }
}

module Geometry {
    struct Point {
        x: Int;
        y: Int;
    }
    
    func Distance(p1: Point, p2: Point): Int {
        var dx = p1.x - p2.x;
        var dy = p1.y - p2.y;
        Math::Square(dx) + Math::Square(dy);
    }
}

module Main {
    var origin: Geometry::Point = new Geometry::Point { x = 0; y = 0; };
    
    func Main(): Int {
        var p = new Geometry::Point { x = 3; y = 4; };
        Geometry::Distance(origin, p);
    }
}
```

---

## 9. File Extension

| File Type | Extension |
|-----------|-----------|
| Cygni source code | `.cyg` |
| Flint bytecode | `.fbc` |

---

## 10. Grammar Summary (EBNF)

```ebnf
Program        ::= Module*

Module         ::= 'module' Identifier '{' ModuleContent* '}'
ModuleContent  ::= Module | GlobalVar | Function | Structure

GlobalVar      ::= 'var' Identifier ':' Type '=' Expression ';'

Function       ::= Annotation* 'func' Identifier '(' Parameters? ')' ':' Type FunctionBody
FunctionBody   ::= Block | ';'
Parameters     ::= Parameter (',' Parameter)*
Parameter      ::= Identifier ':' Type

Structure      ::= 'struct' Identifier '{' Field* '}'
Field          ::= Identifier ':' Type ';'

Annotation     ::= '@' Identifier '(' AnnotationArgs? ')'
AnnotationArgs ::= AnnotationArg (',' AnnotationArg)*
AnnotationArg  ::= Identifier '=' String

Block          ::= '{' Statement* '}'

Statement      ::= VarDecl ';'
                 | IfStatement
                 | WhileStatement
                 | Expression ';'

VarDecl        ::= 'var' Identifier (':' Type)? '=' Expression

IfStatement    ::= 'if' '(' Expression ')' Block ('else' (IfStatement | Block))?
WhileStatement ::= 'while' '(' Expression ')' Block

Expression     ::= Assignment
Assignment     ::= Or ('=' Or)?
Or             ::= And ('||' And)*
And            ::= Not ('&&' Not)*
Not            ::= '!' Not | Equality
Equality       ::= Relation (('==' | '!=') Relation)*
Relation       ::= Additive (('<' | '<=' | '>' | '>=') Additive)?
Additive       ::= Multiplicative (('+' | '-') Multiplicative)*
Multiplicative ::= Unary (('*' | '/') Unary)*
Unary          ::= ('+' | '-' | '!') Unary | Postfix
Postfix        ::= Primary (Call | Member | Scope)*
Call           ::= '(' Arguments? ')'
Member         ::= '.' Identifier
Scope          ::= '::' Identifier
Arguments      ::= Expression (',' Expression)*

Primary        ::= '(' Expression ')'
                 | Block
                 | Literal
                 | Identifier
                 | NewExpression

NewExpression  ::= 'new' QualifiedName '{' FieldInit* '}'
FieldInit      ::= Identifier '=' Expression ';'

Literal        ::= Integer | Float | Character | String | 'true' | 'false'
QualifiedName  ::= Identifier ('::' Identifier)*
Type           ::= QualifiedName TypeArgs?
TypeArgs       ::= '[' Type (',' Type)* ']'
```
