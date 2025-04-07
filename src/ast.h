#ifndef AST_H
#define AST_H

#include <memory>
#include <vector>
#include <string>

enum class VarType { INT, STRING };

class ASTNode {
public:
    virtual ~ASTNode() = default;
};

class ProgramNode : public ASTNode {
public:
    std::vector<std::unique_ptr<ASTNode>> statements;
};

class VarDeclNode : public ASTNode {
public:
    VarDeclNode(VarType type, std::string name, std::unique_ptr<ASTNode> value)
        : type(type), name(std::move(name)), value(std::move(value)) {}
    
    VarType type;
    std::string name;
    std::unique_ptr<ASTNode> value;
};

class AssignNode : public ASTNode {
public:
    AssignNode(std::string name, std::unique_ptr<ASTNode> value)
        : name(std::move(name)), value(std::move(value)) {}
    
    std::string name;
    std::unique_ptr<ASTNode> value;
};

class IntLiteral : public ASTNode {
public:
    IntLiteral(int value) : value(value) {}
    int value;
};

class StrLiteral : public ASTNode {
public:
    StrLiteral(std::string value) : value(std::move(value)) {}
    std::string value;
};

#endif