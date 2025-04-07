#pragma once
#include <string>
#include <variant>

struct Token {
    enum Type {
        Eof, Int, StringType, Ident, Equal, Semicolon,
        IntLiteral, StrLiteral, Error
    };
    
    Type type;
    std::string lexeme;
    int line;
};

class Lexer {
public:
    Lexer(const std::string& source);
    Token nextToken();
    
private:
    std::string source;
    size_t pos = 0;
    int line = 1;
    
    void skipWhitespace();
    char peek() const;
    char advance();
};