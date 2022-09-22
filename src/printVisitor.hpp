#ifndef PRINT_VISITOR_H
#define PRINT_VISITOR_H

#include "ast.hpp"

class PrintVisitor : public Visitor {
    void visit(CallSmtAST* E);
    void visit(BinaryAST* E);
    void visit(PrototypeAST* E);
    void visit(FunctionAST* E);
    void visit(GroupingAST* E);
    void visit(NumberAST* E);
    void visit(VariableAST* E);
    void visit(AssignAST* E);
    void visit(ReturnAST* E);
    void visit(IfAST* E);
};

#endif