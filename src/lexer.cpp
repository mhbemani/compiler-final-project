#include "lexer.h"
#include <cctype>
#include <iostream>
 //  in tekenizing char we have problems and its desabled for now
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
        size_t start = pos; ///////////////////////////////
        pos++;
        column++;
        while (pos < source.size() && std::isalnum(source[pos])) {
            pos++;
            column++;
        }
        std::string lexeme = source.substr(start, pos - start);
        
        if (lexeme == "int") return {Token::Int, "", line, column};
        if (lexeme == "string") return {Token::StringType, "", line, column};
        if (lexeme == "bool") return {Token::Bool, "", line, column};
        if (lexeme == "true" || lexeme == "false") return {Token::BoolLiteral, lexeme, line, column};
        if (lexeme == "float") return {Token::Float, "", line, column};
        if (lexeme == "char") return {Token::Char, "", line, column};
        if (lexeme == "if") return {Token::If, "", line, column};
        if (lexeme == "else") return {Token::Else, "", line, column};
        // if (lexeme.size() == 1) return {Token::CharLiteral, lexeme, line, column};

        //          add other keywords           //

        return {Token::Ident, lexeme, line, column};
    }
    
    
    //  float  and int
    if (std::isdigit(c) || c == '-' || c == '+') {
        
        
        size_t start = pos;
        if (c == '+' || c == '-') {
            pos++;
            column++;
            if(!std::isdigit(peek())){
                // pos++; column++;
                if (peek() == '=') {
                    pos++; column++;
                    return {Token::MinusEqual, "", line, column};
                }
                return {Token::Minus, "", line, column};
            }else{
                bool hasDot = false;
                // if source[pos] != digit return syntax err
                while (pos < source.size() && (std::isdigit(source[pos]) || source[pos] == '.')) {
                    if (source[pos] == '.') hasDot = true;
                    pos++;
                    column++;
                }
                std::string num = source.substr(start, pos - start);
                // std::cout << num << std::endl;

                return {hasDot ? Token::FloatLiteral : Token::IntLiteral, num, line, column};
            }
            // c = source[pos]; // update c after consuming sign
        }else{
            bool hasDot = false;
            // if source[pos] != digit return syntax err
            while (pos < source.size() && (std::isdigit(source[pos]) || source[pos] == '.')) {
                if (source[pos] == '.') hasDot = true;
                pos++;
                column++;
            }
            std::string num = source.substr(start, pos - start);
            // std::cout << num << std::endl;

            return {hasDot ? Token::FloatLiteral : Token::IntLiteral, num, line, column};
        }
    }
    
    if (c == '=') {
        pos++; column++;
        if (peek() == '=') {
            pos++; column++;
            return {Token::EqualEqual, "", line, column};
        }
        return {Token::Equal, "", line, column};
    }

    if (c == '<') {
        pos++; column++;
        if (peek() == '=') {
            pos++; column++;
            return {Token::LessEqual, "", line, column};
        }
        return {Token::Less, "", line, column};
    }

    if (c == '>') {
        pos++; column++;
        if (peek() == '=') {
            pos++; column++;
            return {Token::GreaterEqual, "", line, column};
        }
        return {Token::Greater, "", line, column};
    }

    if (c == '!') {
        pos++; column++;
        if (peek() == '=') {
            pos++; column++;
            return {Token::NotEqual, "", line, column};
        }
        // not defined yet. in case of use will return error
    }

    if (c == '&') {
        pos++; column++;
        if (peek() == '&') {
            pos++; column++;
            return {Token::And, "", line, column};
        }
        // not defined yet. in case of use will return error
    }

    if (c == '|') {
        pos++; column++;
        if (peek() == '|') {
            pos++; column++;
            return {Token::Or, "", line, column};
        }
        // not defined yet. in case of use will return error
    }

    if (c == ',') {
        pos++;
        column++;
        return {Token::Comma, "", line, column};
    }

    if (c == '(') {
        pos++;
        column++;
        return {Token::LeftParen, "", line, column};
    }
    
    if (c == ')') {
        pos++;
        column++;
        return {Token::RightParen, "", line, column};
    }

    if (c == '{') {
        pos++;
        column++;
        return {Token::LeftBrace, "", line, column};
    }

    if (c == '}') {
        pos++;
        column++;
        return {Token::RightBrace, "", line, column};
    }

    if (c == ';') {
        pos++;
        column++;
        return {Token::Semicolon, "", line, column};
    }
    
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
    
    // std::cout << "reached here ...\n";
    if (c == '+') {
        pos++; column++;
        if (peek() == '=') {
            pos++; column++;
            return {Token::PlusEqual, "", line, column};
        }
        return {Token::Plus, "", line, column};
    }
    
    if (c == '*') {
        pos++; column++;
        if (peek() == '=') {
            pos++; column++;
            return {Token::StarEqual, "", line, column};
        }
        return {Token::Star, "", line, column};
    }
    
    if (c == '/') {
        pos++; column++;
        if (peek() == '=') {
            pos++; column++;
            return {Token::SlashEqual, "", line, column};
        }
        return {Token::Slash, "", line, column};
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