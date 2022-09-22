#ifndef TOKEN_TYPE_H
#define TOKEN_TYPE_H

enum TokenType {
    // Single-character tokens.
    TOK_LEFT_PAREN,
    TOK_RIGHT_PAREN,
    TOK_LEFT_BRACE,
    TOK_RIGHT_BRACE,
    TOK_COMMA,
    TOK_DOT,
    TOK_MINUS,
    TOK_PLUS,
    TOK_SEMICOLON,
    TOK_SLASH,
    TOK_STAR,

    // One or two character tokens.
    TOK_BANG,
    TOK_BANG_EQUAL,
    TOK_EQUAL,
    TOK_EQUAL_EQUAL,
    TOK_GREATER,
    TOK_GREATER_EQUAL,
    TOK_LESS,
    TOK_LESS_EQUAL,

    // Literals.
    TOK_IDENTIFIER,
    TOK_STRING,
    TOK_NUMBER,

    // Keywords.
    TOK_AND,
    TOK_ELSE,
    TOK_FALSE,
    TOK_FUN,
    TOK_FOR,
    TOK_IF,
    TOK_NIL,
    TOK_OR,
    TOK_PRINT,
    TOK_RETURN,
    TOK_TRUE,
    TOK_VAR,
    TOK_EOF,
};

#endif