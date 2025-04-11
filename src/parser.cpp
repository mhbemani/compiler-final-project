#include "parser.h"
#include <stdexcept>

Parser::Parser(Lexer& lexer) : lexer(lexer) {
    currentToken = lexer.nextToken();
    peekToken = lexer.nextToken();
}
//  switches to the next Token
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
    if (currentToken.type == Token::Int || currentToken.type == Token::StringType
        || currentToken.type == Token::Bool || currentToken.type == Token::Float
        || currentToken.type == Token::Char) {
        return parseVarDecl();
    }
    if (currentToken.type == Token::Ident) {
        return parseAssignment();
    }
    throw std::runtime_error("Unexpected token in statement");
}

std::unique_ptr<ASTNode> Parser::parseVarDecl() {
    VarType type;
    if (currentToken.type == Token::Int) type = VarType::INT;
    else if (currentToken.type == Token::StringType) type = VarType::STRING;
    else if (currentToken.type == Token::Bool) type = VarType::BOOL;
    else if (currentToken.type == Token::Float) type = VarType::FLOAT;
    else if (currentToken.type == Token::Char) type = VarType::CHAR;
    else throw std::runtime_error("Unknown type in variable declaration");
    advance(); // Consume type
    
    if (currentToken.type != Token::Ident) {
        throw std::runtime_error("Expected identifier after type");
    }
    std::string name = currentToken.lexeme;
    advance(); // Consume ident
    /* block below has been changed */
    // if (currentToken.type != Token::Equal) {
    //     throw std::runtime_error("Expected '=' in variable declaration");
    // }
    if (currentToken.type == Token::Comma) {
        return parseVarDeclMultiVariable(type, name);
    }
    if(currentToken.type == Token::Semicolon){
        advance();
        return std::make_unique<VarDeclNode>(type, name, nullptr); ///////////////////
    }
    if (currentToken.type != Token::Equal) {
        throw std::runtime_error("Expected '=' in variable declaration");
    }

    advance(); // Consume =
    //extractiong value
    std::unique_ptr<ASTNode> value;
    if (type == VarType::INT && currentToken.type == Token::IntLiteral) {
        value = std::make_unique<IntLiteral>(std::stoi(currentToken.lexeme));
        advance();
    } else if (type == VarType::STRING && currentToken.type == Token::StrLiteral) {
        value = std::make_unique<StrLiteral>(currentToken.lexeme);
        advance();
    } else if (type == VarType::BOOL && currentToken.type == Token::BoolLiteral) {
        value = std::make_unique<BoolLiteral>(currentToken.lexeme == "true");
        advance();
    } else if (type == VarType::FLOAT) {
        if (currentToken.type == Token::FloatLiteral) {
            value = std::make_unique<FloatLiteral>(std::stof(currentToken.lexeme));
        } else if (currentToken.type == Token::IntLiteral) {
            value = std::make_unique<FloatLiteral>(static_cast<float>(std::stoi(currentToken.lexeme)));
        } else {
            throw std::runtime_error("Type mismatch: expected float literal");
        }
        advance();
    } else if (type == VarType::CHAR && currentToken.type == Token::CharLiteral) {
        value = std::make_unique<CharLiteral>(currentToken.lexeme[0]);
        advance();
    } else {
        throw std::runtime_error("Type mismatch in variable declaration");
    }
    advance(); // Consume ;
    
    return std::make_unique<VarDeclNode>(type, name, std::move(value));
}

///////////////////////////////

/* the one below is equal to the one above, the definition will be checked in sema.cpp */
//std::unique_ptr<ASTNode> Parser::parseVarDecWithoutAssignment() 

std::unique_ptr<ASTNode> Parser::parseVarDeclMultiVariable(VarType type, std::string name) {
    int counter = 1; // might not be neccesary
    std::vector<std::string> IdentNames;
    IdentNames.push_back(name);
    while(true){
        advance();
        if (currentToken.type != Token::Ident) {
            throw std::runtime_error("Expected identifier after Comma");
        }
        name = currentToken.lexeme;
        IdentNames.push_back(name);
        advance();
        if(currentToken.type == Token::Equal) break;
        else if(currentToken.type != Token::Comma) {
            throw std::runtime_error("Expected Token::Equal or Token::Comma after Identifier");
        }
    }
    // now the current token is = 
    advance();
    std::unique_ptr<ASTNode> value;
    if (type == VarType::INT && currentToken.type == Token::IntLiteral) {
        value = std::make_unique<IntLiteral>(std::stoi(currentToken.lexeme));
        advance();
        if(currentToken.type == Token::Comma) return parseVarDeclMultiBoth(type, std::move(value), IdentNames);
    } 
    else if (type == VarType::STRING && currentToken.type == Token::StrLiteral) {
        value = std::make_unique<StrLiteral>(currentToken.lexeme);
        advance();
        if(currentToken.type == Token::Comma) return parseVarDeclMultiBoth(type, std::move(value), IdentNames);
    } 
    else {
        throw std::runtime_error("Type mismatch in variable declaration");
    }
    // if (currentToken.type != Token::Semicolon) {
    //     throw std::runtime_error("Expected ';' after variable declaration");
    // }
    // advance(); // Consume ;
    std::vector<std::unique_ptr<VarDeclNode>> declarations;
    size_t i = 0;

    while (i < IdentNames.size()) {
        std::unique_ptr<ASTNode> valueCopy; // New value for each VarDeclNode
        if (auto* intLit = dynamic_cast<IntLiteral*>(value.get())) {
            valueCopy = std::make_unique<IntLiteral>(intLit->value); // Deep copy for int
        } else if (auto* strLit = dynamic_cast<StrLiteral*>(value.get())) {
            valueCopy = std::make_unique<StrLiteral>(strLit->value); // Deep copy for string
        } else {
            throw std::runtime_error("Unsupported value type in declaration");
        }
        declarations.emplace_back(std::make_unique<VarDeclNode>(type, IdentNames[i], std::move(valueCopy)));
        i++;
    }
    // if(currentToken.type == Token::Comma){
    //     auto insideVarDecl = parseVarDecl();
    //     if (auto multiVarDecl = dynamic_cast<MultiVarDeclNode*>(insideVarDecl)) {
    //         // Handle multiple variable declarations
    //         for (auto& decl : multiVarDecl->declarations) {
    //             // generateVarDecl(decl.get());
    //         }
    //     } else if (auto varDecl = dynamic_cast<VarDeclNode*>(node)) {
    //         // generateVarDecl(varDecl);
    //     }
    // }
    if (currentToken.type != Token::Semicolon) {
        throw std::runtime_error("Expected ';' after variable declaration");
    }
    advance(); // Consume ;
return std::make_unique<MultiVarDeclNode>(std::move(declarations));
}

std::unique_ptr<ASTNode> Parser::parseVarDeclMultiBoth(VarType type, std::unique_ptr<ASTNode> value, std::vector<std::string> IdentNames){
    std::vector<std::unique_ptr<VarDeclNode>> declarations;
    declarations.emplace_back(std::make_unique<VarDeclNode>(type, IdentNames[0], std::move(value)));
    int i = 1;
    while(i < IdentNames.size()){
        if(currentToken.type != Token::Comma){
            throw std::runtime_error("Type mismatch in variable declaration");
        }
        advance();
        if(currentToken.type != Token::IntLiteral){
            throw std::runtime_error("Type mismatch in variable declaration");
        }
        value = std::make_unique<IntLiteral>(std::stoi(currentToken.lexeme));
        declarations.emplace_back(std::make_unique<VarDeclNode>(type, IdentNames[i], std::move(value)));
        advance();
        i++;
    }
    if (currentToken.type != Token::Semicolon) {
        throw std::runtime_error("Expected ';' after variable declaration");
    }
    advance(); // Consume ;
    return std::make_unique<MultiVarDeclNode>(std::move(declarations));
}

///////////////////////////////
std::unique_ptr<ASTNode> Parser::parseAssignment() {
    std::string name = currentToken.lexeme;
    auto tempType = currentToken.type;
    advance(); // Consume ident
    
    if (currentToken.type != Token::Equal) {
        throw std::runtime_error("Expected '=' in assignment");
    }
    advance(); // Consume =
    
    std::unique_ptr<ASTNode> value;
    if (currentToken.type == Token::IntLiteral || currentToken.type == Token::FloatLiteral) {
        float float_val = std::stof(currentToken.lexeme); // Works for both "4" and "4.0"
        if(tempType == Token::Float){
            value = std::make_unique<FloatLiteral>(float_val);
        } else {
            value = std::make_unique<IntLiteral>(static_cast<int>(float_val));
        }
        advance();
    } else if (currentToken.type == Token::StrLiteral) {
        value = std::make_unique<StrLiteral>(currentToken.lexeme); advance();
    } else if (currentToken.type == Token::BoolLiteral) {
        value = std::make_unique<BoolLiteral>(currentToken.lexeme == "true"); advance();
    }/* else if (currentToken.type == Token::FloatLiteral) {
        value = std::make_unique<FloatLiteral>(std::stof(currentToken.lexeme)); advance();
    } */else if (currentToken.type == Token::CharLiteral) {
        value = std::make_unique<CharLiteral>(currentToken.lexeme[0]); advance();
    } else {
        throw std::runtime_error("Invalid value in assignment");
    }
    advance(); // Consume ;
    
    return std::make_unique<AssignNode>(name, std::move(value));
}

//std::unique_ptr<ASTNode> Parser::parseIf()

//std::unique_ptr<ASTNode> Parser::parseLoop()

//std::unique_ptr<ASTNode> Parser::parseIf()

//std::unique_ptr<ASTNode> Parser::parseUnaryOperator()  x++

//std::unique_ptr<ASTNode> Parser::parseCompositeOperator() // +/ =/

//std::unique_ptr<ASTNode> Parser::parseFor()

//std::unique_ptr<ASTNode> Parser::parseForEach()

//std::unique_ptr<ASTNode> Parser::parsePrint()
    // String::functions
//std::unique_ptr<ASTNode> Parser::parseConcat()
    // math::functions
//std::unique_ptr<ASTNode> Parser::parsePow()

//std::unique_ptr<ASTNode> Parser::parseabs()
    //array::functions
//std::unique_ptr<ASTNode> Parser::parseLength()

//std::unique_ptr<ASTNode> Parser::parseMin()

//std::unique_ptr<ASTNode> Parser::parseMax()

//std::unique_ptr<ASTNode> Parser::parseIndex()

//std::unique_ptr<ASTNode> Parser::parseMultiply()

//std::unique_ptr<ASTNode> Parser::parseAdd()

//std::unique_ptr<ASTNode> Parser::parseSubtract()

//std::unique_ptr<ASTNode> Parser::parseDivide()

//std::unique_ptr<ASTNode> Parser::parseArrayCalculations() // adding s m d aNumber with all the elements of an array

//std::unique_ptr<ASTNode> Parser::parseTryCatch()

//std::unique_ptr<ASTNode> Parser::parseOneLineIf()

//std::unique_ptr<ASTNode> Parser::parseSwithCase()
