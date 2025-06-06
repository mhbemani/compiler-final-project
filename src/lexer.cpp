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
    
    if (std::isalpha(c) || c == '_') {
        size_t start = pos; ///////////////////////////////
        pos++;
        column++;
        if(c == '_' || std::isdigit(source[pos])) throw std::runtime_error("Parser-Error: Names can not begin with numbers.");
        while (pos < source.size() && (std::isalnum(source[pos]) || source[pos] == '_')) {
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
        if (lexeme == "print") return {Token::Print, "", line, column};
        if (lexeme == "for") return {Token::For, "", line, column};
        if (lexeme == "foreach") return {Token::Foreach, "", line, column};
        if (lexeme == "in") return {Token::In, "", line, column};
        if (lexeme == "concat") return {Token::Concat, "", line, column};
        if (lexeme == "pow") return {Token::Pow, "", line, column};
        if (lexeme == "abs") return {Token::Abs, "", line, column};
        if (lexeme == "array") return {Token::Array, "", line, column};
        if (lexeme == "length") return {Token::Length, lexeme, line, column - int(lexeme.size())};
        if (lexeme == "min") return {Token::Min, lexeme, line, column - int(lexeme.size())};
        if (lexeme == "max") return {Token::Max, lexeme, line, column - int(lexeme.size())};
        if (lexeme == "index") return {Token::Index, lexeme, line, column - int(lexeme.size())};
        if (lexeme == "multiply") return {Token::Multiply, lexeme, line, column - int(lexeme.size())};
        if (lexeme == "add") return {Token::Add, lexeme, line, column - int(lexeme.size())};
        if (lexeme == "subtract") return {Token::Subtract, lexeme, line, column - int(lexeme.size())};
        if (lexeme == "divide") return {Token::Divide, lexeme, line, column - int(lexeme.size())};
        if (lexeme == "try") return {Token::Try, "", line, column};
        if (lexeme == "catch") return {Token::Catch, "", line, column};
        if (lexeme == "error") return {Token::Error, "", line, column};
        if (lexeme == "match") return {Token::Match, "", line, column};
        

        //          add other keywords           //

        return {Token::Ident, lexeme, line, column};
    }
    
    if (c == '-' || c == '+' || std::isdigit(c)) {
        bool hasDot = false;
        bool isSigned = (c == '-' || c == '+');
        size_t start = pos;
        std::string lexeme;

        // Handle sign
        if (isSigned) {
            lexeme += c;
            pos++;
            column++;

            if (peek() == '(') {
                pos++;
                column++;
                return {Token::negLeftParen, lexeme + "=", line, column - 1};
            }
            // Check for -= or +=
            if (peek() == '=') {
                pos++;
                column++;
                return {c == '-' ? Token::MinusEqual : Token::PlusEqual, lexeme + "=", line, column - 1};
            }
            
            if (c == '-' && peek() == '-') {
                pos++;
                column++;
                return {Token::MinusMinus, lexeme + "-", line, column - 1};
            }
            
            if (c == '+' && peek() == '+') {
                pos++;
                column++;
                return {Token::PlusPlus, lexeme + "+", line, column - 1};
            }

            if (c == '-' && peek() == '>') {
                pos++;
                column++;
                return {Token::Arrow, lexeme + "+", line, column - 1};
            }
            
            if (pos >= source.size() || !std::isdigit(source[pos])) {
                return {c == '-' ? Token::Minus : Token::Plus, lexeme, line, column - 1};
            }
        }

        // Collect digits and optional dot
        while (pos < source.size() && (std::isdigit(source[pos]) || source[pos] == '.')) {
            if (source[pos] == '.') hasDot = true;
            lexeme += source[pos];
            pos++;
            column++;
        }

        // Determine token type
        if (hasDot) {
            return {Token::FloatLiteral, lexeme, line, column - int(lexeme.size())};
        } else if (isSigned) {
            return {Token::SignedIntLiteral, lexeme, line, column - int(lexeme.size())};
        } else {
            return {Token::IntLiteral, lexeme, line, column - int(lexeme.size())};
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

    if (c == '[') {
        pos++;
        column++;
        return {Token::LeftBracket, "", line, column};
    }

    if (c == '%') {
        pos++; column++;
        if (peek() == '=') {
            pos++; column++;
            return {Token::ModuloEqual, "", line, column};
        }
        return {Token::Modulo, "", line, column};
    }

    if (c == ']') {
        pos++;
        column++;
        return {Token::RightBracket, "", line, column};
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

    if (c == '^') {
        pos++;
        column++;
        return {Token::Xor, "^", line, column};
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

    if (c == '.') {
        pos++;
        column++;
        return {Token::Dot, "", line, column};
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
    
    if (c == ':') {
        pos++;
        column++;
        return {Token::Colon, "", line, column};
    }

    if (c == '_') {
        pos++;
        column++;
        return {Token::Underscore, "", line, column};
    }

    if (c == '?') {
        pos++;
        column++;
        return {Token::Question, "", line, column};
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