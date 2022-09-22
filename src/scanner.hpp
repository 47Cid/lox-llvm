#ifndef SCANNER_H
#define SCANNER_H

#include <iostream>
#include <map>
#include <vector>

#include "common.hpp"
#include "token.hpp"

class Scanner {
   private:
    const std::string Source;
    int Current = 0;
    int Start = 0;
    int Line = 1;
    bool isAtEnd();
    bool match(char c);
    char peek();
    char peekNext();
    char advance();
    void scanToken();
    void xeString();
    void xeNumber();
    void identifier();
    void addToken(TokenType Type);
    void addToken(TokenType Type, std::string Value);
    std::map<std::string, TokenType> Keywords;

   public:
    bool HadError = false;
    std::vector<Token*> Tokens;

    Scanner(const std::string&& Src);
    void scanTokens();
    void printTokens();
    ~Scanner();
};

#endif
