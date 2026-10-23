# Compiler Design Project

This project implements a compiler prototype for a small custom programming language developed as a collaborative **Compiler Design** course project.

The goal of the project was to design a simple programming language and implement the main stages required to process programs written in that language. The language is centered around integer-based computation and supports variable declarations, assignments, arithmetic expressions, conditional logic, and condition-controlled loops.

For example, a program can declare and initialize variables:

```text
int x, y = 5, 10;
```

perform arithmetic and compound assignments:

```text
x = y + 4 * 2;
x += 3;
y %= 4;
```

and evaluate arithmetic expressions containing addition, subtraction, multiplication, division, modulo, exponentiation, and parentheses:

```text
x = (y + 3) * 2;
y = x ^ 2;
```

The language also supports comparison operators such as `==`, `!=`, `<`, `>`, `<=`, and `>=`, as well as compound conditions using `and` and `or`.

Conditional execution follows an `if` / `elif` / `else` structure with explicit `begin` and `end` blocks. For example:

```text
if x >= y : begin
    x += 1;
end

elif x == y : begin
    y += 2;
end
```

Condition-controlled loops are represented using the custom `loopc` construct:

```text
loopc x < 10 : begin
    x += 1;
end
```

The compiler processes source code through the major stages of a traditional compilation pipeline. It first performs lexical analysis to recognize keywords, identifiers, numeric literals, operators, and punctuation. The resulting token stream is then parsed according to the grammar of the custom language and represented as an Abstract Syntax Tree.

The project also includes semantic-analysis logic for checking the use of identifiers and detecting invalid variable usage. After the program has been analyzed, the code-generation component is designed to translate the AST into **LLVM Intermediate Representation (LLVM IR)**.

The LLVM-based backend contains support for translating integer arithmetic, assignments, comparisons, logical expressions, conditional branches, and loops into LLVM instructions and control-flow blocks.

A small C runtime is also included to provide basic integer input and output functionality for generated programs.

The complete language grammar is available in `grammar.txt`.

## Main Language Capabilities

- Integer variables and declarations
- Variable initialization and assignment
- Compound assignments: `+=`, `-=`, `*=`, `/=`, `%=`
- Arithmetic operators: `+`, `-`, `*`, `/`, `%`, `^`
- Parenthesized arithmetic expressions
- Comparison operators
- Logical conditions using `and` and `or`
- `if`, `elif`, and `else` control flow
- Condition-controlled loops using `loopc`
- Abstract Syntax Tree construction
- Semantic analysis
- LLVM IR generation
- Basic runtime input/output support

## Contributors

This project was developed collaboratively by:

- Niloofar Riahi
- Arnoosha Najafi
- Negin Ghorbani
