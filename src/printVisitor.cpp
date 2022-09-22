#include "printVisitor.hpp"

void PrintVisitor::visit(CallSmtAST* E) {
    std::cout << "(CALL :";
    std::cout << E->Callee << ")";
}

void PrintVisitor::visit(PrototypeAST* E) {
    std::cout << "(PROTO: ";
    std::cout << E->Name;
    std::cout << ")";
}

void PrintVisitor::visit(FunctionAST* E) {
    std::cout << "(FUNCTION: ";
    std::cout << "name: ";
    std::cout << E->Proto->getName() << " ";
    for (auto& Smt : E->BodyStmts)
        Smt->accept(*this);
    std::cout << ")";
}

void PrintVisitor::visit(BinaryAST* E) {
    std::cout << "(BINARY: ";
    std::cout << E->Op->Lexeme;
    E->Left->accept(*this);
    E->Right->accept(*this);
    std::cout << ")";
}
void PrintVisitor::visit(GroupingAST* E) {
    std::cout << "(GROUPING: ";
    E->Expr->accept(*this);
    std::cout << ")";
}

void PrintVisitor::visit(VariableAST* E) {
    std::cout << "(VARIABLE: ";
    std::cout << E->Name->Lexeme << " ";
    std::cout << ")";
}

void PrintVisitor::visit(NumberAST* E) {
    std::cout << "(NUMBER: ";
    std::cout << E->Value;
    std::cout << ")";
}

void PrintVisitor::visit(StringAST* E) {
    std::cout << "(STRING: ";
    std::cout << E->Value;
    std::cout << ")";
}

void PrintVisitor::visit(AssignAST* E) {
    std::cout << "(ASSIGN: ";
    std::cout << "name: " << E->Name->Lexeme << " ";
    std::cout << "value :";
    E->Value->accept(*this);
    std::cout << ")";
}

void PrintVisitor::visit(IfAST* E) {
    std::cout << "(IF: "
              << "then : ";
    for (auto& Smt : E->Then)
        Smt->accept(*this);
    std::cout << "else: ";
    for (auto& Smt : E->Else)
        Smt->accept(*this);
    std::cout << ")";
}

void PrintVisitor::visit(ReturnAST* E) {
    std::cout << "(RETURN: ";
    E->Value->accept(*this);
    std::cout << ")";
}
