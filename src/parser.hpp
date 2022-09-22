#ifndef PARSER_H
#define PARSER_H

#include <memory>
#include <vector>

#include "ast.hpp"
#include "printVisitor.hpp"
#include "scanner.hpp"
#include "token.hpp"

class Parser {
   private:
    PrintVisitor printer;
    int Current = 0;
    bool PanicMode = false;

    // Helper functions
    bool match(TokenType Type);
    bool check(TokenType Type);
    Token* advance();
    Token* peek();
    Token* previous();
    Token* consume(TokenType Type, std::string Message);
    std::unique_ptr<SmtAST> error(std::string Message);

    // Ast functions
    std::unique_ptr<SmtAST> primary();
    std::unique_ptr<SmtAST> call();
    std::unique_ptr<SmtAST> unary();
    std::unique_ptr<SmtAST> factor();
    std::unique_ptr<SmtAST> term();
    std::unique_ptr<SmtAST> comparison();
    std::unique_ptr<SmtAST> equality();
    std::unique_ptr<SmtAST> assignment();
    std::unique_ptr<SmtAST> expression();
    std::unique_ptr<PrototypeAST> proto();
    std::unique_ptr<FunctionAST> funDecl();
    std::unique_ptr<AssignAST> varDecl();
    std::unique_ptr<SmtAST> expressionStatement();
    std::unique_ptr<IfAST> ifStatement();
    std::unique_ptr<ReturnAST> returnStatement();
    std::unique_ptr<SmtAST> statement();
    std::unique_ptr<SmtAST> declStmt();

   public:
    bool HadError = false;
    std::vector<Token*> Tokens;

    std::vector<std::unique_ptr<SmtAST>> ParsedStmts;

    bool isAtEnd();
    Parser(const std::vector<Token*> Tokens) : Tokens(Tokens) { printer = PrintVisitor(); }
    std::unique_ptr<SmtAST> parseAST();
    void parseStmts();
    void printAST();
    ~Parser();
};

#endif