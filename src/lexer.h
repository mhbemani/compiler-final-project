#pragma once
#include <string>
#include <variant>

struct Token {
    enum Type {
        Eof, 
        Int, 
        StringType, 
        Ident,
        Bool,
        /* signs begin */ 
        Equal, 
        DoubleQuotation,
        Semicolon,
        Comma,
        /* signs end */
        IntLiteral, 
        StrLiteral, 
        BoolLiteral,
        Error
    };
    
    Type type;
    std::string lexeme;
    int line;
    int column;  // ← New: Character position in line
};

class Lexer {
public:
    Lexer(const std::string& source);
    
    Token nextToken();
    Token peekToken();  // ← New: LL(1) lookahead
    
    void error(const std::string& msg);

private:
    std::string source;
    size_t pos = 0;
    int line = 1;
    int column = 1;
    
    void skipWhitespace();
    void skipComments();
    char peek() const;
    char advance();
};