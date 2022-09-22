#include "parser.hpp"

std::unique_ptr<SmtAST> Parser::error(std::string Message) {
    HadError = true;
    // Don't print more errors if parser is already in panic mode
    if (!PanicMode)
        std::cerr << Message << std::endl;
    PanicMode = true;
    return nullptr;
}

Token* Parser::consume(TokenType Type, std::string Message) {
    if (check(Type))
        return advance();
    else {
        error(Message);
        return nullptr;
    }
}

Token* Parser::peek() {
    return Tokens.at(Current);
}

Token* Parser::previous() {
    return Tokens.at(Current - 1);
}

bool Parser::isAtEnd() {
    return peek()->Type == TOK_EOF;
}

bool Parser::check(TokenType Type) {
    if (isAtEnd()) return false;
    return peek()->Type == Type;
}

Token* Parser::advance() {
    if (!isAtEnd()) Current++;
    return previous();
}

bool Parser::match(TokenType Type) {
    if (check(Type)) {
        advance();
        return true;
    }
    return false;
}

std::unique_ptr<PrototypeAST> Parser::proto() {
    Token* Name = consume(TOK_IDENTIFIER, "Expected function name.");
    std::string FnName = Name->Lexeme;

    consume(TOK_LEFT_PAREN, "Expect '(' after name.");
    std::vector<std::string> ArgNames;

    if (!check(TOK_RIGHT_PAREN)) {
        do {
            Token* Arg = consume(TOK_IDENTIFIER, "Expect parameter name.");
            ArgNames.push_back(Arg->Lexeme);
        } while (match(TOK_COMMA));
    }

    consume(TOK_RIGHT_PAREN, "Expect ')' after name.");
    return std::make_unique<PrototypeAST>(FnName, ArgNames);
}

std::unique_ptr<SmtAST> Parser::call() {
    std::unique_ptr<SmtAST> Expr = primary();
    if (peek()->Type != TOK_LEFT_PAREN)
        return Expr;

    Token* Name = previous();

    advance();
    std::vector<std::unique_ptr<SmtAST>> Args;

    if (!check(TOK_RIGHT_PAREN)) {
        do {
            std::unique_ptr<SmtAST> Arg = expression();
            Args.push_back(move(Arg));
        } while (match(TOK_COMMA));
    }
    consume(TOK_RIGHT_PAREN, "Expect ')' after parameters.");

    return std::make_unique<CallSmtAST>(Name->Lexeme, std::move(Args));
}

std::unique_ptr<SmtAST> Parser::primary() {
    if (match(TOK_NUMBER)) {
        return std::make_unique<NumberAST>(std::stod(previous()->Lexeme));
    }

    if (match(TOK_IDENTIFIER)) {
        return std::make_unique<VariableAST>(previous());
    }

    if (match(TOK_LEFT_PAREN)) {
        std::unique_ptr<SmtAST> Expr = expression();
        consume(TOK_RIGHT_PAREN, "Expect ')' after expression.");
        return std::make_unique<GroupingAST>(std::move(Expr));
    }
    return error("Error parsing primary expression");
}

std::unique_ptr<SmtAST> Parser::factor() {
    std::unique_ptr<SmtAST> Expr = call();
    while (match(TOK_SLASH) || match(TOK_STAR)) {
        Token* Op = previous();
        std::unique_ptr<SmtAST> Right = call();
        Expr = std::make_unique<BinaryAST>(std::move(Expr), Op, std::move(Right));
    }
    return Expr;
}

std::unique_ptr<SmtAST> Parser::term() {
    std::unique_ptr<SmtAST> Expr = factor();
    while (match(TOK_MINUS) || match(TOK_PLUS)) {
        Token* Op = previous();
        std::unique_ptr<SmtAST> Right = factor();
        Expr = std::make_unique<BinaryAST>(std::move(Expr), Op, std::move(Right));
    }

    return Expr;
}

std::unique_ptr<SmtAST> Parser::comparison() {
    std::unique_ptr<SmtAST> Expr = term();

    while (match(TOK_GREATER) || match(TOK_GREATER_EQUAL) || match(TOK_LESS) || match(TOK_LESS_EQUAL)) {
        Token* Op = previous();
        std::unique_ptr<SmtAST> Right = term();
        Expr = std::make_unique<BinaryAST>(std::move(Expr), Op, std::move(Right));
    }

    return Expr;
}

std::unique_ptr<SmtAST> Parser::equality() {
    std::unique_ptr<SmtAST> Expr = comparison();

    while (match(TOK_BANG_EQUAL) || match(TOK_EQUAL_EQUAL)) {
        Token* Op = previous();
        std::unique_ptr<SmtAST> Right = comparison();
        Expr = std::make_unique<BinaryAST>(std::move(Expr), Op, std::move(Right));
    }

    return Expr;
}

std::unique_ptr<SmtAST> Parser::assignment() {
    std::unique_ptr<SmtAST> E = equality();

    if (match(TOK_EQUAL)) {
        std::unique_ptr<SmtAST> Value = assignment();

        std::unique_ptr<VariableAST> NewE = std::unique_ptr<VariableAST>(dynamic_cast<VariableAST*>(E.release()));
        if (NewE == nullptr) {
            std::cerr << "Dynamic casting failed";
            return nullptr;
        }
        return std::make_unique<AssignAST>(NewE->Name, std::move(Value), false);
    }
    return E;
}

std::unique_ptr<SmtAST> Parser::expression() {
    std::unique_ptr<SmtAST> E = assignment();
    if (!E) {
        return error("Error parsing expression");
    }
    return E;
}

std::unique_ptr<SmtAST> Parser::expressionStatement() {
    std::unique_ptr<SmtAST> E = expression();
    if (!E) {
        return nullptr;
    }
    consume(TOK_SEMICOLON, "Expect ';' after expression statement.");
    return E;
}

std::unique_ptr<IfAST> Parser::ifStatement() {
    consume(TOK_LEFT_PAREN, "Expect '(' after 'if'.");
    std::unique_ptr<SmtAST> Cond = expression();
    consume(TOK_RIGHT_PAREN, "Expect ')' after if condition.");

    consume(TOK_LEFT_BRACE, "Expect '{' before definition.");
    std::vector<std::unique_ptr<SmtAST>> ThenStmts;
    while (!check(TOK_RIGHT_BRACE) && !isAtEnd()) {
        ThenStmts.push_back(statement());
    }
    consume(TOK_RIGHT_BRACE, "Expect '}' after definition.");

    std::vector<std::unique_ptr<SmtAST>> ElseStmts;
    if (match(TOK_ELSE)) {
        consume(TOK_LEFT_BRACE, "Expect '{' before definition.");
        while (!check(TOK_RIGHT_BRACE) && !isAtEnd()) {
            ElseStmts.push_back(statement());
        }
        consume(TOK_RIGHT_BRACE, "Expect '}' after definition.");
    }

    return std::make_unique<IfAST>(std::move(Cond), std::move(ThenStmts), std::move(ElseStmts));
}

std::unique_ptr<ReturnAST> Parser::returnStatement() {
    std::unique_ptr<SmtAST> E = expression();
    if (!E) {
        error("Error parsing return statement");
        return nullptr;
    }
    consume(TOK_SEMICOLON, "Expect ';' after return statement.");
    return std::make_unique<ReturnAST>(std::move(E));
}

std::unique_ptr<FunctionAST> Parser::funDecl() {
    std::unique_ptr<PrototypeAST> protoE = proto();
    if (!protoE)
        return nullptr;

    consume(TOK_LEFT_BRACE, "Expect '{' before definition.");
    std::vector<std::unique_ptr<SmtAST>> BodyStmts;
    while (!check(TOK_RIGHT_BRACE) && !isAtEnd()) {
        BodyStmts.push_back(statement());
    }
    consume(TOK_RIGHT_BRACE, "Expect '}' after definition.");

    return std::make_unique<FunctionAST>(std::move(protoE), std::move(BodyStmts));
}

std::unique_ptr<AssignAST> Parser::varDecl() {
    Token* Name = consume(TOK_IDENTIFIER, "Expect variable name.");
    std::unique_ptr<SmtAST> Value;
    if (match(TOK_EQUAL)) {
        Value = expression();
    }
    consume(TOK_SEMICOLON, "Expect ';' after variable declaration.");
    return std::make_unique<AssignAST>(Name, std::move(Value), true);
}

std::unique_ptr<SmtAST> Parser::statement() {
    if (match(TOK_IF)) return ifStatement();
    if (match(TOK_RETURN)) return returnStatement();
    if (match(TOK_VAR)) return varDecl();
    return expressionStatement();
}

std::unique_ptr<SmtAST> Parser::declStmt() {
    if (match(TOK_FUN)) return funDecl();
    return error("Error parsing declaration statement");
}

void Parser::parseStmts() {
    while (!isAtEnd()) {
        // Reset the panic mode
        PanicMode = false;
        // Parse Statments
        std::unique_ptr<SmtAST> Expr = declStmt();
        if (Expr) {
            ParsedStmts.push_back(std::move(Expr));
        }
    }
}

void Parser::printAST() {
    for (auto& Smt : ParsedStmts) {
        Smt->accept(printer);
    }
}

Parser::~Parser() {
}