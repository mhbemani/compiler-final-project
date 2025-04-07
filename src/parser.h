#ifndef PARSER_H
#define PARSER_H

#include "ast.h"
#include "lexer.h"
#include <memory>
#include <stdexcept>  // For std::runtime_error

class Parser {
public:
    Parser(Lexer& lexer);
    std::unique_ptr<ProgramNode> parseProgram();
    
private:
    Lexer& lexer;
    Token currentToken;
    Token peekToken;
    
    // Core parsing
    void advance();
    std::unique_ptr<ASTNode> parseStatement();
    std::unique_ptr<ASTNode> parseVarDecl();
    std::unique_ptr<ASTNode> parseAssignment();

    // Critical additions
    [[noreturn]] void error(const std::string& msg, const Token& token);
    void consume(Token::Type type);  // For required tokens (e.g. semicolons)

    // Optional future extensions
    /*
    // For expressions (when needed)
    std::unique_ptr<ExprNode> parseExpression();
    std::unique_ptr<ExprNode> parseAdditive();
    
    // For blocks (when needed)
    std::unique_ptr<BlockNode> parseBlock();
    */
};

#endif