#include <string>   // For std::string
#include <memory>
#include <vector>

// base class
class ExprAST {
    public:
        virtual ~ExprAST() = default;
};

//numeric literals
class NumberExprAST : public ExprAST {
    double value;
    public:
    NumberExprAST(double val) :value(val) {}
};
// for referencing a variable
class VariablExprAST : public ExprAST {
    std::string Name;
public:
    VariablExprAST(const std::string &Name): Name(Name) {}
};
// expression class for binary operation
class BinaryExprAST : public ExprAST {
    char op;
    std::unique_ptr<ExprAST> LHS,RHS;
public:
    BinaryExprAST(char op,std::unique_ptr<ExprAST> LHS,std::unique_ptr<ExprAST> RHS): op(op),LHS(std::move(LHS)),RHS(std::move(RHS)) {}
};
//func prototype ast
class PrototypeAst {
    std::string Name;
    std::vector<std::string> args;
    public:
        PrototypeAst(const std::string &Name,std::vector<std::string> args): Name(Name),args(std::move(args)) {}
        const std::string &getName() const { return Name;}
};

// no type as by default the type retuned is a double
class FunctionAST {
    std::unique_ptr<PrototypeAst> proto;
    std::unique_ptr<ExprAST> body;
    public:
        FunctionAST(std::unique_ptr<PrototypeAst> proto,std::unique_ptr<ExprAST> body):
            proto(std::move(proto)),body(std::move(body)) {}
};

/// CallExprAST - Expression class for function calls.
class CallExprAST : public ExprAST {
  std::string Callee;
  std::vector<std::unique_ptr<ExprAST>> Args;

public:
  CallExprAST(const std::string &Callee,
              std::vector<std::unique_ptr<ExprAST>> Args)
      : Callee(Callee), Args(std::move(Args)) {}
};

