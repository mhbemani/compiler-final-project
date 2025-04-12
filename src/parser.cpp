#include "parser.h"
#include <stdexcept>
#include <iostream>

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
            // add ; check for all parsestatement just like ident
        return parseVarDecl();
    }
    if (currentToken.type == Token::Ident) {
        auto tempreturn = parseAssignment();
        if (currentToken.type != Token::Semicolon) {
            throw std::runtime_error("Expected ';' after assignment");
        }
        advance();
        return tempreturn;
    }
    if (currentToken.type == Token::If) {
        return parseIfStatement();
    }
    if (currentToken.type == Token::Print) {  // New case
        advance(); // Consume 'print'
        if (currentToken.type != Token::LeftParen) {
            throw std::runtime_error("Expected '(' after 'print'");
        }
        advance(); // Consume '('
        auto expr = parseExpression(); // Parse the expression inside print()
        if (currentToken.type != Token::RightParen) {
            throw std::runtime_error("Expected ')' after print expression");
        }
        advance(); // Consume ')'
        if (currentToken.type != Token::Semicolon) {
            throw std::runtime_error("Expected ';' after print statement");
        }
        advance(); // Consume ';'
        return std::make_unique<PrintNode>(std::move(expr));
    }
    if (currentToken.type == Token::For || currentToken.type == Token::Foreach) {
        return parseLoop();
    }
    
    

     std::cout << (currentToken.type == Token::Semicolon) << std::endl;
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

    if (currentToken.type == Token::Comma) {
        return parseVarDeclMultiVariable(type, name);
    }
    if (currentToken.type == Token::Semicolon) {
        advance();
        return std::make_unique<VarDeclNode>(type, name, nullptr);
    }
    if (currentToken.type != Token::Equal) {
        throw std::runtime_error("Expected '=' in variable declaration");
    }
    advance(); // Consume '='

    // Parse expression for value
    std::unique_ptr<ASTNode> value = parseExpression();

    if (currentToken.type != Token::Semicolon) {
        throw std::runtime_error("Expected ';' after variable declaration");
    }
    advance(); // Consume ';'
    
    return std::make_unique<VarDeclNode>(type, name, std::move(value));
}

std::unique_ptr<ASTNode> Parser::parseExpression() {
    auto left = parsePrimary();

    while (currentToken.type == Token::Plus || currentToken.type == Token::Minus ||
           currentToken.type == Token::Star || currentToken.type == Token::Slash || 
           currentToken.type == Token::EqualEqual || currentToken.type == Token::LessEqual ||
           currentToken.type == Token::NotEqual || currentToken.type == Token::Greater ||
           currentToken.type == Token::GreaterEqual || currentToken.type == Token::Less ||
           currentToken.type == Token::And || currentToken.type == Token::Or
            // ||currentToken.type == Token::IntLiteral
        ) {
        BinaryOp op;
        if (currentToken.type != Token::IntLiteral){
            switch (currentToken.type) {
                case Token::Plus: op = BinaryOp::ADD; break;
                case Token::Minus: op = BinaryOp::SUBTRACT; break;
                case Token::Star: op = BinaryOp::MULTIPLY; break;
                case Token::Slash: op = BinaryOp::DIVIDE; break;
                case Token::EqualEqual: op = BinaryOp::EQUAL; break;
                case Token::LessEqual: op = BinaryOp::LESS_EQUAL; break;
                case Token::NotEqual: op = BinaryOp::NOT_EQUAL; break;
                case Token::Greater: op = BinaryOp::GREATER; break;
                case Token::GreaterEqual: op = BinaryOp::GREATER_EQUAL; break;
                case Token::Less: op = BinaryOp::LESS; break;
                case Token::And: op = BinaryOp::AND; break;
                case Token::Or: op = BinaryOp::OR; break;
                // case Token::IntLiteral: BinaryOp::ADD; break;
                default: throw std::runtime_error("Unknown binary operator");
            }
    
            advance(); // consume operator
            auto right = parsePrimary();
    
            left = std::make_unique<BinaryOpNode>(op, std::move(left), std::move(right));
        }else{
            op = BinaryOp::ADD;
            auto right = std::make_unique<IntLiteral>(std::stoi(currentToken.lexeme));
            advance(); // consume the int literal
            left = std::make_unique<BinaryOpNode>(op, std::move(left), std::move(right));
        }
    }

    return left;
}

std::unique_ptr<ASTNode> Parser::parsePrimary() {
    if (currentToken.type == Token::IntLiteral) {
        auto node = std::make_unique<IntLiteral>(std::stoi(currentToken.lexeme));
        advance();
        return node;
    } else if (currentToken.type == Token::StrLiteral) {
        auto node = std::make_unique<StrLiteral>(currentToken.lexeme);
        advance();
        return node;
    } else if (currentToken.type == Token::BoolLiteral) {
        auto node = std::make_unique<BoolLiteral>(currentToken.lexeme == "true");
        advance();
        return node;
    } else if (currentToken.type == Token::FloatLiteral) {
        auto node = std::make_unique<FloatLiteral>(std::stof(currentToken.lexeme));
        advance();
        return node;
    } else if (currentToken.type == Token::CharLiteral) {
        auto node = std::make_unique<CharLiteral>(currentToken.lexeme[0]);
        advance();
        return node;
    } else if (currentToken.type == Token::Ident) {
        auto node = std::make_unique<VarRefNode>(currentToken.lexeme); // You need a VariableRefNode in your AST
        advance();
        return node;
    } else if (currentToken.type == Token::Concat) {
        advance();
        if (currentToken.type != Token::LeftParen) {
            throw std::runtime_error("Expected '(' after 'concat'");
        }
        advance();
        auto left = parseExpression();
        if (!left) {
            throw std::runtime_error("Expected first argument in concat");
        }
        if (currentToken.type != Token::Comma) {
            throw std::runtime_error("Expected ',' after first argument in concat");
        }
        advance();
        auto right = parseExpression();
        if (!right) {
            throw std::runtime_error("Expected second argument in concat");
        }
        if (currentToken.type != Token::RightParen) {
            throw std::runtime_error("Expected ')' after concat arguments");
        }
        advance();
        return std::make_unique<ConcatNode>(std::move(left), std::move(right));
    } else {
        throw std::runtime_error("Expected primary expression");
    }
}
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
        throw std::runtime_error("Type mismatch in variable declaration(2)");
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
            throw std::runtime_error("Type mismatch in variable declaration(3)");
        }
        advance();
        if(currentToken.type != Token::IntLiteral){
            throw std::runtime_error("Type mismatch in variable declaration(4)");
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

std::unique_ptr<ASTNode> Parser::parseAssignment() {
    std::string name = currentToken.lexeme;
    auto tempType = currentToken.type;
    advance(); // Consume ident
    
    BinaryOp compoundOp;
    bool isCompound = false;

    // Handle =, +=, -=, *=, /=
    if (currentToken.type == Token::Equal) {
        advance();
    } else if (currentToken.type == Token::PlusEqual) {
        compoundOp = BinaryOp::ADD;
        isCompound = true;
        advance();
    } else if (currentToken.type == Token::MinusEqual) {
        compoundOp = BinaryOp::SUBTRACT;
        isCompound = true;
        advance();
    } else if (currentToken.type == Token::StarEqual) {
        compoundOp = BinaryOp::MULTIPLY;
        isCompound = true;
        advance();
    } else if (currentToken.type == Token::SlashEqual) {
        compoundOp = BinaryOp::DIVIDE;
        isCompound = true;
        advance();
    } else {
        throw std::runtime_error("Expected '=' or compound assignment operator");
    }
   
    auto value = parseExpression(); // new

    // if (currentToken.type != Token::Semicolon) {
    //     throw std::runtime_error("Expected ';' after assignment");
    // }
    // advance(); // Consume ;

    if (isCompound) {
        return std::make_unique<CompoundAssignNode>(name, compoundOp, std::move(value));
    } else {
        return std::make_unique<AssignNode>(name, std::move(value));
    }
}

std::unique_ptr<ASTNode> Parser::parseIfStatement() {
    advance(); // Consume 'if'

    if (currentToken.type != Token::LeftParen) {
        throw std::runtime_error("Expected '(' after 'if'");
    }
    advance(); // Consume '('

    auto condition = parseExpression(); // Parse the condition (e.g., "true", "a > b")

    if (currentToken.type != Token::RightParen) {
        throw std::runtime_error("Expected ')' after condition");
    }
    advance(); // Consume ')'

    if (currentToken.type != Token::LeftBrace) {
        throw std::runtime_error("Expected '{' after if condition");
    }
    auto thenBlock = parseBlock(); // Parse the "then" block

    std::unique_ptr<ASTNode> elseBlock = nullptr;
    if (currentToken.type == Token::Else) {
        advance(); // Consume 'else'
        if (currentToken.type == Token::If) {
            elseBlock = parseIfStatement(); // Recurse for "else if"
        } else if (currentToken.type == Token::LeftBrace) {
            elseBlock = parseBlock(); // Plain "else" block
        } else {
            throw std::runtime_error("Expected 'if' or '{' after 'else'");
        }
    }

    return std::make_unique<IfElseNode>(std::move(condition), std::move(thenBlock), std::move(elseBlock));
}

std::unique_ptr<BlockNode> Parser::parseBlock() {
    if (currentToken.type != Token::LeftBrace) {
        throw std::runtime_error("Expected '{' to start block");
    }
    advance(); // consume '{'

    auto block = std::make_unique<BlockNode>();

    while (currentToken.type != Token::RightBrace) {
        block->statements.push_back(parseStatement());
    }

    advance(); // consume '}'
    return block;
}

std::unique_ptr<ASTNode> Parser::parseLoop() {
    bool isForeach = (currentToken.type == Token::Foreach);
    advance(); // Consume 'for' or 'foreach'
    if (currentToken.type != Token::LeftParen) throw std::runtime_error("Expected '(' after loop keyword");
    advance(); // Consume '('

    if (isForeach) {
        if (currentToken.type != Token::Ident) throw std::runtime_error("Expected identifier in foreach");
        std::string varName = currentToken.lexeme;
        advance(); // Consume varName
        if (currentToken.type != Token::In) throw std::runtime_error("Expected 'in' in foreach");
        advance(); // Consume 'in'
        if (currentToken.type != Token::Ident) throw std::runtime_error("Expected collection identifier after 'in'");
        std::string collectionName = currentToken.lexeme;
        advance(); // Consume collectionName
        if (currentToken.type != Token::RightParen) throw std::runtime_error("Expected ')' after foreach");
        advance(); // Consume ')'
        auto body = parseBlock();
        return std::make_unique<LoopNode>(varName, collectionName, std::move(body));
    } else {
        std::unique_ptr<ASTNode> init = nullptr;
        if (currentToken.type == Token::Int) {
            init = parseVarDecl(); // e.g., int i = 0
        } else if (currentToken.type == Token::Ident) {
            init = parseAssignment(); // e.g., i = 0
        } 
        // else if (currentToken.type != Token::Semicolon) {
        //     throw std::runtime_error("Expected declaration, assignment, or ';' in for init");
        // }
        if (currentToken.type != Token::Semicolon) throw std::runtime_error("Expected ';' after init");
        advance(); // Consume ';'
        auto condition = currentToken.type == Token::Semicolon ? nullptr : parseExpression();
        if (currentToken.type != Token::Semicolon) throw std::runtime_error("Expected ';' after condition");
        advance(); // Consume ';'
        auto update = currentToken.type == Token::RightParen ? nullptr : parseAssignment(); // expression changed to assignment
        if (currentToken.type != Token::RightParen) throw std::runtime_error("Expected ')' after update");
        advance(); // Consume ')'
        auto body = parseBlock();
        return std::make_unique<LoopNode>(std::move(init), std::move(condition), std::move(update), std::move(body));
    }
}

//std::unique_ptr<ASTNode> Parser::parseUnaryOperator()  x++
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
