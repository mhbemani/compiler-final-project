#include "lexer.h"
#include <cctype>
#include <iostream>

Lexer::Lexer(const std::string& source) : source(source) {}

void Lexer::skipWhitespace() {
    while (pos < source.size() && std::isspace(source[pos])) {
        if (source[pos] == '\n') {
            line++;
            column = 1;
        } else {
            column++;
        }
        pos++;
    }
}

void Lexer::skipComments() {
    if (pos + 1 >= source.size()) return;
    
    // Check for single-line comment (//)
    if (source[pos] == '/' && source[pos + 1] == '/') {
        pos += 2;
        column += 2;
        // Skip until end of line
        while (pos < source.size() && source[pos] != '\n') {
            pos++;
            column++;
        }
        if (pos < source.size()) {  // consume the newline
            pos++;
            line++;
            column = 1;
        }
    }
    // Check for multi-line comment (/* ... */)
    else if (source[pos] == '/' && source[pos + 1] == '*') {
        pos += 2;
        column += 2;
        bool commentClosed = false;
        
        while (pos + 1 < source.size()) {
            if (source[pos] == '*' && source[pos + 1] == '/') {
                pos += 2;
                column += 2;
                commentClosed = true;
                break;
            }
            if (source[pos] == '\n') {
                line++;
                column = 1;
            } else {
                column++;
            }
            pos++;
        }
        
        if (!commentClosed) {
            error("Unterminated multi-line comment");
        }
    }
}

Token Lexer::nextToken() {
    skipWhitespace();
    skipComments();
    skipWhitespace();
    
    if (pos >= source.size()) return {Token::Eof, "", line, column};
    
    char c = source[pos];
    size_t token_start_column = column;
    
    if (std::isalpha(c)) {
        size_t start = pos;
        pos++;
        column++;
        while (pos < source.size() && std::isalnum(source[pos])) {
            pos++;
            column++;
        }
        std::string lexeme = source.substr(start, pos - start);
        
        if (lexeme == "int") return {Token::Int, "", line, column};
        if (lexeme == "String") return {Token::StringType, "", line, column};
        if (lexeme == "bool") return {Token::Bool, "", line, column};
        if (lexeme == "true" || lexeme == "false") return {Token::BoolLiteral, lexeme, line, column};

        //          add other keywords           //

        return {Token::Ident, lexeme, line, column};
    }
    
    if (std::isdigit(c)) {
        size_t start = pos;
        pos++;
        column++;
        while (pos < source.size() && std::isdigit(source[pos])) {
            pos++;
            column++;
        }
        return {Token::IntLiteral, source.substr(start, pos - start), line, column};
    }
    


    if (c == '=') {
        pos++;
        column++;
        return {Token::Equal, "", line, column};
    }

    if (c == ',') {
        pos++;
        column++;
        return {Token::Comma, "", line, column};
    }
    
    if (c == ';') {
        pos++;
        column++;
        return {Token::Semicolon, "", line, column};
    }
    
    // std::cout << "reached here ...\n";
    if (c == '"') {
        pos++;
        column++;
        size_t start = pos;
        while (pos < source.size() && source[pos] != '"') {
            if (source[pos] == '\n') {
                line++;
                column = 1;
            } else {
                column++;
            }
            pos++;
        }
        
        
        if (pos >= source.size()) return {Token::Error, "Unterminated string", line, column};
        std::string value = source.substr(start, pos - start);
        pos++;
        column++;
        return {Token::StrLiteral, value, line, column};
    }
    
    //        add other operators or anything needed like these           //

    pos++;
    column++;
    return {Token::Error, "Unexpected character", line, column};
}

Token Lexer::peekToken() {
    size_t saved_pos = pos;
    int saved_line = line;
    int saved_column = column;
    
    Token token = nextToken();
    
    pos = saved_pos;
    line = saved_line;
    column = saved_column;
    
    return token;
}

void Lexer::error(const std::string& msg) {
    // You can implement error reporting here
    throw std::runtime_error(msg + " at line " + std::to_string(line) + 
                            ", column " + std::to_string(column));
}

char Lexer::peek() const {
    return pos < source.size() ? source[pos] : '\0';
}

char Lexer::advance() {
    if (pos >= source.size()) return '\0';
    char c = source[pos++];
    if (c == '\n') {
        line++;
        column = 1;
    } else {
        column++;
    }
    return c;
}