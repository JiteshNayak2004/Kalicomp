#include "ast.cpp"
#include <map>
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
// binary expression parsing
static std::map<char,int> BinopPrecedence;
static int GetTokPrecedence() {
    if (!isascii(CurTok))
        return -1;
    int tokprec=BinopPrecedence[CurTok];
    if (tokprec<=0) return -1;
    return tokprec;
}
static std::unique_ptr<ExprAST> ParseBinOpRHS(int ExprPrec,std::unique_ptr<ExprAST> LHS) {

    while(true) {
        int TokPRec=GetTokPrecedence();
        if (TokPRec<ExprPrec)
            return LHS;
        int BinOp=CurTok;
        getNextToken();

        auto RHS=ParsePrimary();
        if (!RHS)
            return nullptr;

        int NextPrec=GetTokPrecedence();
        if (TokPRec<NextPrec) {
            RHS=ParseBinOpRHS(TokPRec+1, std::move(RHS));
            if (!RHS)
                return nullptr;
        }

        LHS=std::make_unique<BinaryExprAST>(BinOp,std::move(LHS),std::move(RHS));
    }
}
// we support just 4 operator <,+,-,* and apparently map can be assigned values in main
// the rhs is like a+b*c+d which is a then [+,b] then [*,c]
// the sequence of pairs is parsed by ParseBinOpRHS
static std::unique_ptr<ExprAST> ParseExpression() {
    auto LHS=ParsePrimary();
    if(!LHS)
        return nullptr;
    return ParseBinOpRHS(0,std::move(LHS));
}

/// prototype
///   ::= id '(' id* ')'
static std::unique_ptr<PrototypeAst> ParsePrototype() {
  if (CurTok != tok_identifier)
    return LogErrorP("Expected function name in prototype");

  std::string FnName = Identifierstr;
  getNextToken();

  if (CurTok != '(')
    return LogErrorP("Expected '(' in prototype");

  std::vector<std::string> ArgNames;
  while (getNextToken() == tok_identifier)
    ArgNames.push_back(Identifierstr);
  if (CurTok != ')')
    return LogErrorP("Expected ')' in prototype");

  // success.
  getNextToken(); // eat ')'.

  return std::make_unique<PrototypeAst>(FnName, std::move(ArgNames));
}

/// definition ::= 'def' prototype expression
static std::unique_ptr<FunctionAST> ParseDefinition() {
  getNextToken(); // eat def.
  auto Proto = ParsePrototype();
  if (!Proto)
    return nullptr;

  if (auto E = ParseExpression())
    return std::make_unique<FunctionAST>(std::move(Proto), std::move(E));
  return nullptr;
}


/// toplevelexpr ::= expression
static std::unique_ptr<FunctionAST> ParseTopLevelExpr() {
  if (auto E = ParseExpression()) {
    // Make an anonymous proto.
    auto Proto = std::make_unique<PrototypeAst>("__anon_expr",
                                                std::vector<std::string>());
    return std::make_unique<FunctionAST>(std::move(Proto), std::move(E));
  }
  return nullptr;
}

/// external ::= 'extern' prototype
static std::unique_ptr<PrototypeAst> ParseExtern() {
  getNextToken(); // eat extern.
  return ParsePrototype();
}

static void HandleDefinition() {
  if (ParseDefinition()) {
    fprintf(stderr, "Parsed a function definition.\n");
  } else {
    // Skip token for error recovery.
    getNextToken();
  }
}

static void HandleExtern() {
  if (ParseExtern()) {
    fprintf(stderr, "Parsed an extern\n");
  } else {
    // Skip token for error recovery.
    getNextToken();
  }
}

static void HandleTopLevelExpression() {
  // Evaluate a top-level expression into an anonymous function.
  if (ParseTopLevelExpr()) {
    fprintf(stderr, "Parsed a top-level expr\n");
  } else {
    // Skip token for error recovery.
    getNextToken();
  }
}
/// top ::= definition | external | expression | ';'
static void MainLoop() {
  while (true) {
    fprintf(stderr, "ready> ");
    switch (CurTok) {
    case tok_eof:
      return;
    case ';': // ignore top-level semicolons.
      getNextToken();
      break;
    case tok_def:
      HandleDefinition();
      break;
    case tok_extern:
      HandleExtern();
      break;
    default:
      HandleTopLevelExpression();
      break;
    }
  }
}

// main driver code
int main() {
  // Install standard binary operators.
  // 1 is lowest precedence.
  BinopPrecedence['<'] = 10;
  BinopPrecedence['+'] = 20;
  BinopPrecedence['-'] = 20;
  BinopPrecedence['*'] = 40; // highest.

  // Prime the first token.
  fprintf(stderr, "ready> ");
  getNextToken();

  // Run the main "interpreter loop" now.
  MainLoop();

  return 0;
}

