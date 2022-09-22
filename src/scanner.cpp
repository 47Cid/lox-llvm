#include "scanner.hpp"

Scanner::Scanner(const std::string&& Src) : Source(std::move(Src)) {
    Keywords.insert(std::pair<std::string, TokenType>("else", TOK_ELSE));
    Keywords.insert(std::pair<std::string, TokenType>("fun", TOK_FUN));
    Keywords.insert(std::pair<std::string, TokenType>("if", TOK_IF));
    Keywords.insert(std::pair<std::string, TokenType>("return", TOK_RETURN));

    Keywords.insert(std::pair<std::string, TokenType>("var", TOK_VAR));
    Keywords.insert(std::pair<std::string, TokenType>("number", TOK_TYPE_NUM));
    Keywords.insert(std::pair<std::string, TokenType>("string", TOK_TYPE_STRING));
}

bool Scanner::match(char Expected) {
    if (isAtEnd())
        return false;
    if (Source[Current] != Expected)
        return false;
    Current++;
    return true;
}

bool Scanner::isAtEnd() {
    return Current >= Source.length();
}

char Scanner::peek() {
    if (isAtEnd())
        return '\0';
    return Source[Current];
}

char Scanner::peekNext() {
    if (Current + 1 >= Source.length())
        return '\0';
    return Source[Current + 1];
}

char Scanner::advance() {
    return Source[Current++];
}

void Scanner::addToken(TokenType Type) {
    std::string Text;

    Text = Source.substr(Start, Current - Start);

    Tokens.push_back(new Token(Type, Text, Line));
}

void Scanner::addToken(TokenType Type, std::string Value) {
    Tokens.push_back(new Token(Type, Value, Line));
}

void Scanner::xeString() {
    while (peek() != '"' && !isAtEnd()) {
        if (peek() == '\n')
            Line++;
        advance();
    }

    if (isAtEnd()) {
        std::cerr << "Unterminated string.";
        HadError = true;
        return;
    }

    // The closing ".
    advance();

    // Trim the surrounding quotes.
    std::string value = Source.substr(Start + 1, Current - Start - 2);
    addToken(TOK_STRING, value);
}

void Scanner::xeNumber() {
    while (isdigit(peek()))
        advance();

    // Look for a fractional part.
    if (peek() == '.' && isdigit(peekNext())) {
        // Consume the "."
        advance();

        while (isdigit(peek()))
            advance();
    }

    addToken(TOK_NUMBER, Source.substr(Start, Current - Start));
}

void Scanner::identifier() {
    while (isalnum(peek()))
        advance();

    std::string Text = Source.substr(Start, Current - Start);

    auto It = Keywords.find(Text);

    TokenType Type = It->second;
    if (It == Keywords.end())
        Type = TOK_IDENTIFIER;
    addToken(Type);
}

void Scanner::scanToken() {
    char c = advance();
    switch (c) {
        case '(':
            addToken(TOK_LEFT_PAREN);
            break;
        case ')':
            addToken(TOK_RIGHT_PAREN);
            break;
        case '{':
            addToken(TOK_LEFT_BRACE);
            break;
        case '}':
            addToken(TOK_RIGHT_BRACE);
            break;
        case ',':
            addToken(TOK_COMMA);
            break;
        case '-':
            addToken(TOK_MINUS);
            break;
        case '+':
            addToken(TOK_PLUS);
            break;
        case ';':
            addToken(TOK_SEMICOLON);
            break;
        case ':':
            addToken(TOK_COLON);
            break;
        case '*':
            addToken(TOK_STAR);
            break;

        case '!':
            addToken(match('=') ? TOK_BANG_EQUAL : TOK_BANG);
            break;
        case '=':
            addToken(match('=') ? TOK_EQUAL_EQUAL : TOK_EQUAL);
            break;
        case '<':
            addToken(match('=') ? TOK_LESS_EQUAL : TOK_LESS);
            break;
        case '>':
            addToken(match('=') ? TOK_GREATER_EQUAL : TOK_GREATER);
            break;
        case '/':
            if (match('/')) {
                // A comment goes until the end of the line.
                while (peek() != '\n' && !isAtEnd())
                    advance();
            } else {
                addToken(TOK_SLASH);
            }
            break;

        case '"':
            xeString();
            break;

        case ' ':
        case '\r':
        case '\t':
            // Ignore whitespace.
            break;

        case '\n':
            Line++;
            break;

        default:
            if (isdigit(c)) {
                xeNumber();
            } else if (isalpha(c)) {
                identifier();
            } else {
                std::cerr << "Unexpected character" << std::endl;
                HadError = true;
            }
            break;
    }
}

void Scanner::scanTokens() {
    while (!isAtEnd()) {
        // We are at the beginning of the next lexeme.
        Start = Current;
        scanToken();
    }

    Tokens.push_back(new Token(TOK_EOF, "", Line));
}

void Scanner::printTokens() {
    for (auto Token : Tokens) {
        std::cout << Token->toString() << std::endl;
    }
}

Scanner::~Scanner() {}
