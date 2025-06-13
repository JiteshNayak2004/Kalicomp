#include "ast.cpp"
#include "lexer.cpp"
#include <error.h>
#include <string>   // For std::string
#include <memory>
#include <vector>

static int CurTok;
static int getNextToken() {
  return CurTok = gettok();
}

/// LogError* - These are little helper functions for error handling.
std::unique_ptr<ExprAST> LogError(const char *Str) {
  fprintf(stderr, "Error: %s\n", Str);
  return nullptr;
}
std::unique_ptr<PrototypeAst> LogErrorP(const char *Str) {
  LogError(Str);
  return nullptr;
}

static std::unique_ptr<ExprAST> ParseExpression();

//numeric literals

static std::unique_ptr<ExprAST> parsenumberexpr() {
    auto result=std::make_unique<NumberExprAST>(NumVal);
    getNextToken();
    return std::move(result);
}
// parse parenthesis
static std::unique_ptr<ExprAST> ParseParenExpr() {
    getNextToken();
    auto V = ParseExpression();
    if (!V)
        return nullptr;
    if (CurTok != ')')
        return LogError("expected ')'");
    getNextToken(); // eat ).
    return V;
}

// parse variable references and function calls

static std::unique_ptr<ExprAST> parseidentifierexpr() {
    std::string IdName=Identifierstr;
    getNextToken();
    if (CurTok!='(')
        return std::make_unique<VariablExprAST>(IdName);

    getNextToken();
    std::vector<std::unique_ptr<ExprAST>> args;
    if (CurTok!=')') {
        while (true) {
            if (auto arg=ParseExpression())
                args.push_back(std::move(arg));
            else
             return nullptr;
            if (CurTok==')')
                break;
            if (CurTok!=',')
                return LogError("expected ')' or ',' did'nt find");
            getNextToken();
        }
    }
    getNextToken();
    return std::make_unique<CallExprAST>(IdName,std::move(args));
}

/// primary
///   ::= identifierexpr
///   ::= numberexpr
///   ::= parenexpr
static std::unique_ptr<ExprAST> ParsePrimary() {
  switch (CurTok) {
  default:
    return LogError("unknown token when expecting an expression");
  case tok_identifier:
    return parseidentifierexpr();
  case tok_number:
    return parsenumberexpr();
  case '(':
    return ParseParenExpr();
  }
}

