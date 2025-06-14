# Kalicomp: A Barebones Kaleidoscope Compiler
ps: gemini wrote this beautiful documentation below this should help anyone to debug update and enhance the current compiler frontend 

Kalicomp is a minimal compiler implementation for the Kaleidoscope language. This documentation outlines its core components and the parsing flow, providing a guide for understanding, updating, or enhancing the compiler frontend.

---

## Notes on Kaleidoscope

* **Type Inference:** Kaleidoscope, by default, infers all variables to be of type `double`.
* **Lexer:** The lexer takes raw text input and tokenizes it, associating metadata with each token. Invalid tokens are reported by their ASCII value.
* **Abstract Syntax Tree (AST):** The AST captures the language's structure in a way that simplifies subsequent compiler stages. Each language construct is represented by a dedicated AST object.
* **Core Language Constructs:** Kaleidoscope primarily deals with expressions, prototypes, and function objects.

---

## Abstract Syntax Tree (AST) Structure

The AST is composed of several specialized nodes:

* **`NumberExprAST`**: Represents numerical literal values.
* **`VariableExprAST`**: Represents a variable, storing its identifier string.
* **`BinaryExprAST`**: Represents a binary operation, holding the operator and pointers to the left and right expressions.
* **`CallExprAST`**: Represents a function call, capturing the function's name and its arguments.
* **`PrototypeAST`**: Represents a function prototype, holding the function's name and its arguments.
* **`FunctionAST`**: Represents a function definition, containing its prototype and a pointer to its body (an expression).

---

## Parsing Flow

The parsing process in Kalicomp is driven by a main loop that orchestrates the handling of different language constructs.

### `MainLoop()`

This is the top-level loop responsible for continuously reading and parsing input.

1.  It begins by calling `getNextToken()` to retrieve the initial token.
2.  It then continuously inspects `CurTok` (the current token):
    * If `tok_eof` (end of file) is encountered, the loop terminates.
    * If a semicolon (`;`) is found, it's consumed, effectively ignoring empty lines.
    * If `def` is encountered, it signifies a function definition, and `HandleDefinition()` is invoked.
    * If `extern` is encountered, `HandleExtern()` is called.
    * For all other cases, it's assumed to be a regular expression (e.g., `1 + 2 * 3`), and `HandleTopLevelExpression()` is called.

### Handling Top-Level Expressions

(`HandleTopLevelExpression` $\rightarrow$ `ParseTopLevelExpr` $\rightarrow$ `ParseExpression`)

When you enter an expression, the `MainLoop` directs it to `ParseTopLevelExpr`. The primary role of `ParseTopLevelExpr` is to call `ParseExpression()`, which is where the actual parsing of arithmetic expressions takes place.

### The Heart of Expression Parsing: `ParseExpression()`, `ParsePrimary()`, and `ParseBinOpRHS()`

These functions are critical for constructing the expression tree, especially in correctly respecting operator precedence.

#### `ParsePrimary()`

This function is responsible for parsing the simplest, "atomic" building blocks of an expression that do not involve binary operators. It examines `CurTok` and acts accordingly:

* If `tok_number`, it calls `ParseNumberExpr()` to create a `NumberExprAST`.
* If `tok_identifier`, it calls `ParseIdentifierExpr()`. This function further distinguishes between a simple variable (e.g., `x`) and a function call (e.g., `foo(a, b)`).
* If `'('`, it calls `ParseParenExpr()`. This is where recursion plays a role: `ParseParenExpr()` itself calls `ParseExpression()` to parse the content within the parentheses. This mechanism enables correct parsing of expressions like `(1 + 2) * 3`, where `(1 + 2)` is treated as a primary expression.

#### `ParseExpression()`

This is the initial entry point for parsing any expression.

1.  It always starts by calling `ParsePrimary()` to obtain the leftmost, simplest part of the expression. This forms the **LHS (Left-Hand Side)**.
2.  Immediately after, it calls `ParseBinOpRHS(0, std::move(LHS))`. The `0` here represents the "minimum precedence" allowed for operators to be consumed at this stage.

#### `ParseBinOpRHS(int ExprPrec, std::unique_ptr<ExprAST> LHS)`: The Recursive Binary Operator Parsing

This is arguably the most crucial and sophisticated part of the parser, yet elegant in its design. Its objective is to consume any binary operators and their right-hand sides that have a precedence equal to or higher than the `ExprPrec` threshold.

Let's trace the parsing of `1 + 2 * 3` through a simplified walk-through:

1.  **Initial Call:** `ParseBinOpRHS(0, NumberExprAST(1))` (from `ParseExpression`)
    * Current state: `LHS` is `1`. `CurTok` is `+`.
    * `TokPrec = GetTokPrecedence()` for `+` is `20`.
    * Since `20 < 0` is `false`, we proceed to consume `+`.
    * `BinOp = '+'`. `getNextToken()` is called, and `CurTok` becomes `2`.
    * `RHS = ParsePrimary()` is called. `ParsePrimary()` sees `2`, calls `ParseNumberExpr()`, and returns `NumberExprAST(2)`. `getNextToken()` makes `CurTok` become `*`.
    * **Crucial Decision Point (Precedence Check):**
        * `NextPrec = GetTokPrecedence()` for `*` is `40`.
        * `TokPrec (20) < NextPrec (40)` is `true`. This signifies that the `*` operator binds more tightly than our current `+` operator.
        * **Recursive Call:** Because `*` binds tighter, we must allow `*` to take `RHS` (`2`) as its left operand. Therefore, a recursive call is made: `RHS = ParseBinOpRHS(TokPrec + 1 (21), std::move(RHS))`.
        * This means: "Go parse the rest of the expression starting from `2`, but only consider operators with precedence strictly higher than `20` (i.e., `21` and above)."

2.  **Recursive Call:** `ParseBinOpRHS(21, NumberExprAST(2))`
    * Current state: `LHS` is `2`. `CurTok` is `*`.
    * `TokPrec = GetTokPrecedence()` for `*` is `40`.
    * Since `40 < 21` is `false`, we proceed to consume `*`.
    * `BinOp = '*'`. `getNextToken()` is called, and `CurTok` becomes `3`.
    * `RHS = ParsePrimary()` is called. `ParsePrimary()` sees `3`, calls `ParseNumberExpr()`, and returns `NumberExprAST(3)`. `getNextToken()` makes `CurTok` become `tok_eof`.
    * **Crucial Decision Point (Precedence Check):**
        * `NextPrec = GetTokPrecedence()` for `tok_eof` is `-1`.
        * `TokPrec (40) < NextPrec (-1)` is `false`. There are no higher precedence operators.
    * **Merge:** We combine the `LHS` (`2`) with the `RHS` (`3`) using the `*` operator: `LHS = BinaryExprAST('*', NumberExprAST(2), NumberExprAST(3))`. This `LHS` now represents `(2 * 3)`.
    * **Loop Condition:** The `while(true)` loop restarts.
    * `TokPrec = GetTokPrecedence()` for `tok_eof` is `-1`.
    * `-1 < ExprPrec (21)` is `true`. The loop breaks.
    * **Return:** This recursive call returns the AST for `(2 * 3)`.

3.  **Back in the Original Call:** `ParseBinOpRHS(0, NumberExprAST(1))`
    * The `RHS` of this call is now the AST we just received: `BinaryExprAST('*', NumberExprAST(2), NumberExprAST(3))`.
    * **Merge:** We combine the `LHS` (`1`) with this new `RHS` using the `+` operator: `LHS = BinaryExprAST('+', NumberExprAST(1), BinaryExprAST('*', NumberExprAST(2), NumberExprAST(3)))`. This `LHS` now represents `(1 + (2 * 3))`.
    * **Loop Condition:** The `while(true)` loop restarts.
    * `TokPrec = GetTokPrecedence()` for `tok_eof` is `-1`.
    * `-1 < ExprPrec (0)` is `true`. The loop breaks.
    * **Return:** The original `ParseBinOpRHS` call returns the final AST: `BinaryExprAST('+', NumberExprAST(1), BinaryExprAST('*', NumberExprAST(2), NumberExprAST(3)))`.

---
