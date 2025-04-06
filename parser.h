#ifndef PARSER_H
#define PARSER_H

#include "ast.h"
#include "lexer.h"
#include <memory>

class Parser {
public:
    Parser(Lexer& lexer);
    std::unique_ptr<ProgramNode> parseProgram();
    
private:
    Lexer& lexer;
    Token currentToken;
    Token peekToken;
    
    void advance();
    std::unique_ptr<ASTNode> parseStatement();
    std::unique_ptr<ASTNode> parseVarDecl();
    std::unique_ptr<ASTNode> parseAssignment();
};

#endif