#include "parser.h"
#include <stdexcept>

Parser::Parser(Lexer& lexer) : lexer(lexer) {
    currentToken = lexer.nextToken();
    peekToken = lexer.nextToken();
}

void Parser::advance() {
    currentToken = peekToken;
    peekToken = lexer.nextToken();
}

std::unique_ptr<ProgramNode> Parser::parseProgram() {
    auto program = std::make_unique<ProgramNode>();
    
    while (currentToken.type != Token::Eof) {
        auto stmt = parseStatement();
        if (!stmt) {
            throw std::runtime_error("Failed to parse statement");
        }
        program->statements.push_back(std::move(stmt));
    }
    
    return program;
}

std::unique_ptr<ASTNode> Parser::parseStatement() {
    if (currentToken.type == Token::Int || currentToken.type == Token::StringType) {
        return parseVarDecl();
    }
    if (currentToken.type == Token::Ident) {
        return parseAssignment();
    }
    throw std::runtime_error("Unexpected token in statement");
}

std::unique_ptr<ASTNode> Parser::parseVarDecl() {
    VarType type = currentToken.type == Token::Int ? VarType::INT : VarType::STRING;
    advance(); // Consume type
    
    if (currentToken.type != Token::Ident) {
        throw std::runtime_error("Expected identifier after type");
    }
    std::string name = currentToken.lexeme;
    advance(); // Consume ident
    
    if (currentToken.type != Token::Equal) {
        throw std::runtime_error("Expected '=' in variable declaration");
    }
    advance(); // Consume =
    
    std::unique_ptr<ASTNode> value;
    if (type == VarType::INT && currentToken.type == Token::IntLiteral) {
        value = std::make_unique<IntLiteral>(std::stoi(currentToken.lexeme));
        advance();
    } 
    else if (type == VarType::STRING && currentToken.type == Token::StrLiteral) {
        value = std::make_unique<StrLiteral>(currentToken.lexeme);
        advance();
    } 
    else {
        throw std::runtime_error("Type mismatch in variable declaration");
    }
    
    if (currentToken.type != Token::Semicolon) {
        throw std::runtime_error("Expected ';' after variable declaration");
    }
    advance(); // Consume ;
    
    return std::make_unique<VarDeclNode>(type, name, std::move(value));
}

std::unique_ptr<ASTNode> Parser::parseAssignment() {
    std::string name = currentToken.lexeme;
    advance(); // Consume ident
    
    if (currentToken.type != Token::Equal) {
        throw std::runtime_error("Expected '=' in assignment");
    }
    advance(); // Consume =
    
    std::unique_ptr<ASTNode> value;
    if (currentToken.type == Token::IntLiteral) {
        value = std::make_unique<IntLiteral>(std::stoi(currentToken.lexeme));
        advance();
    } 
    else if (currentToken.type == Token::StrLiteral) {
        value = std::make_unique<StrLiteral>(currentToken.lexeme);
        advance();
    } 
    else {
        throw std::runtime_error("Invalid value in assignment");
    }
    
    if (currentToken.type != Token::Semicolon) {
        throw std::runtime_error("Expected ';' after assignment");
    }
    advance(); // Consume ;
    
    return std::make_unique<AssignNode>(name, std::move(value));
}