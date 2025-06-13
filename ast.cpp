#include <string>   // For std::string
#include <memory>
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
