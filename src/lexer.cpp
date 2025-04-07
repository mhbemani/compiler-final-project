#include "lexer.h"
#include <cctype>

Lexer::Lexer(const std::string& source) : source(source) {}

void Lexer::skipWhitespace() {
    while (pos < source.size() && std::isspace(source[pos])) {
        if (source[pos] == '\n') line++;
        pos++;
    }
}

Token Lexer::nextToken() {
    skipWhitespace();
    
    if (pos >= source.size()) return {Token::Eof, "", line};
    
    char c = source[pos];
    if (std::isalpha(c)) {
        size_t start = pos++;
        while (pos < source.size() && std::isalnum(source[pos])) pos++;
        std::string lexeme = source.substr(start, pos - start);
        
        if (lexeme == "int") return {Token::Int, "", line};
        if (lexeme == "String") return {Token::StringType, "", line};
        return {Token::Ident, lexeme, line};
    }
    
    if (std::isdigit(c)) {
        size_t start = pos++;
        while (pos < source.size() && std::isdigit(source[pos])) pos++;
        return {Token::IntLiteral, source.substr(start, pos - start), line};
    }
    
    if (c == '=') {
        pos++;
        return {Token::Equal, "", line};
    }
    
    if (c == ';') {
        pos++;
        return {Token::Semicolon, "", line};
    }
    
    if (c == '"') {
        pos++;
        size_t start = pos;
        while (pos < source.size() && source[pos] != '"') pos++;
        if (pos >= source.size()) return {Token::Error, "Unterminated string", line};
        
        std::string value = source.substr(start, pos - start);
        pos++;
        return {Token::StrLiteral, value, line};
    }
    
    pos++;
    return {Token::Error, "Unexpected character", line};
}