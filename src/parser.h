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
    std::unique_ptr<ASTNode> parseExpression();
    std::unique_ptr<ASTNode> parsePrimary();
    std::unique_ptr<ASTNode> parseIfStatement();
    // std::unique_ptr<ASTNode> parseLogicalExpression();
    std::unique_ptr<ASTNode> parseTryCatch();
    std::unique_ptr<BlockNode> parseBlock();
    std::unique_ptr<ASTNode> parseLoop();
    std::unique_ptr<ASTNode> parseTernary(std::unique_ptr<ASTNode> condition);
    ///////////////////////////////
    
    std::unique_ptr<ASTNode> parseVarDeclMultiVariable(VarType type, std::string name); // int a , b = 10;
    std::unique_ptr<ASTNode> parseVarDeclMultiBoth(VarType type, std::unique_ptr<ASTNode> value, std::vector<std::string> IdentNames);     // int a , b = 10, 12;

    ///////////////////////////////
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