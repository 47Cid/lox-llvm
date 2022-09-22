#ifndef TOKEN_H
#define TOKEN_H

#include <iostream>

#include "tokenType.hpp"

class Token {
public:
    TokenType Type;
    std::string Lexeme;
    int Line;
    Token(TokenType Type, std::string Lexeme, int Line) : Type(Type), Lexeme(Lexeme), Line(Line) {}

    std::string toString() {
        return "Type: " + std::to_string(Type) + " " + "Lexeme: " + Lexeme;
    }
};

#endif