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
        Float,
        Char,
        Print,
        For,
        Foreach,
        In,
        Concat,
        Array,
        Pow,
        Abs,
        Length, 
        Min, 
        Max, 
        Index,
        Multiply, 
        Add, 
        Subtract, 
        Divide,
        Try, 
        Catch, 
        Dot,
        
        /* signs begin */ 
        Equal, 
        DoubleQuotation,
        Semicolon,
        Comma,
        Plus, 
        Minus, 
        Star, 
        Slash,
        PlusEqual, 
        MinusEqual, 
        StarEqual,
        Colon,
        Question,
        LeftBrace,
        RightBrace,
        LeftParen,
        RightParen,
        LeftBracket,
        RightBracket,
        EqualEqual, 
        NotEqual, 
        Less, 
        LessEqual, 
        Greater, 
        GreaterEqual,
        Modulo,
        ModuloEqual,
        And,
        Or,
        Xor,
        PlusPlus,
        MinusMinus,
        Match,
        Arrow,
        Underscore,

        If, 
        Else, 
        SlashEqual,
        /* signs end */
        IntLiteral, 
        StrLiteral, 
        BoolLiteral,
        FloatLiteral,
        CharLiteral,
        SignedIntLiteral,
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