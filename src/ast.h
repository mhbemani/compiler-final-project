#ifndef AST_H
#define AST_H

#include <memory>
#include <vector>
#include <string>

enum class VarType { INT, STRING, BOOL, FLOAT, CHAR };
enum class BinaryOp { ADD, SUBTRACT, MULTIPLY, DIVIDE };

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

class MultiVarDeclNode : public ASTNode {
    public:
        MultiVarDeclNode(std::vector<std::unique_ptr<VarDeclNode>> declarations)
            : declarations(std::move(declarations)) {}
        
        std::vector<std::unique_ptr<VarDeclNode>> declarations;
    };

class AssignNode : public ASTNode {
public:
    AssignNode(std::string name, std::unique_ptr<ASTNode> value)
        : name(std::move(name)), value(std::move(value)) {}
    
    std::string name;
    std::unique_ptr<ASTNode> value;
};

class VarRefNode : public ASTNode {
    public:
        VarRefNode(std::string name) : name(std::move(name)) {}
        std::string name;
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

class BoolLiteral : public ASTNode {
    public:
        BoolLiteral(bool value) : value(value) {}
        bool value;
};

class FloatLiteral : public ASTNode {
    public:
        FloatLiteral(float value) : value(value) {}
        float value;
};

class CharLiteral : public ASTNode {
    public:
        CharLiteral(char value) : value(value) {}
        char value;
};

class BinaryOpNode : public ASTNode {
    public:
        BinaryOpNode(BinaryOp op, std::unique_ptr<ASTNode> left, std::unique_ptr<ASTNode> right)
            : op(op), left(std::move(left)), right(std::move(right)) {}
    
        BinaryOp op;
        std::unique_ptr<ASTNode> left;
        std::unique_ptr<ASTNode> right;
};

class CompoundAssignNode : public ASTNode {
    public:
        CompoundAssignNode(std::string name, BinaryOp op, std::unique_ptr<ASTNode> value)
            : name(std::move(name)), op(op), value(std::move(value)) {}
    
        std::string name;
        BinaryOp op;
        std::unique_ptr<ASTNode> value;
};

#endif