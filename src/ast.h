#ifndef AST_H
#define AST_H

#include <memory>
#include <vector>
#include <string>

enum class VarType { INT, STRING, BOOL, FLOAT, CHAR };
enum class BinaryOp { ADD, SUBTRACT, MULTIPLY, DIVIDE, EQUAL,
     NOT_EQUAL, LESS, LESS_EQUAL, GREATER, GREATER_EQUAL, AND, OR };
// enum class LogicalOp { EQUAL, NOT_EQUAL, LESS, LESS_EQUAL, GREATER, GREATER_EQUAL };
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

struct IfNode : public ASTNode {
    std::unique_ptr<ASTNode> condition;
    std::vector<std::unique_ptr<ASTNode>> ifBody;
    std::vector<std::vector<std::unique_ptr<ASTNode>>> elseBodies;

    IfNode(std::unique_ptr<ASTNode> cond) : condition(std::move(cond)) {}
};

class BlockNode : public ASTNode {
    public:
        std::vector<std::unique_ptr<ASTNode>> statements;
    
        BlockNode() = default;
};
    
class IfElseNode : public ASTNode {
    public:
        // Unified constructor for all if variants
        IfElseNode(std::unique_ptr<ASTNode> condition,
                   std::unique_ptr<ASTNode> then_block,
                   std::unique_ptr<ASTNode> else_block = nullptr)
            : condition(std::move(condition)),
              then_block(std::move(then_block)),
              else_block(std::move(else_block)) {}
    
        // Accessors
        bool hasElseBlock() const { return else_block != nullptr; }
        bool isElseIf() const {
            return hasElseBlock() && dynamic_cast<IfElseNode*>(else_block.get());
        }
    
        std::unique_ptr<ASTNode> condition;
        std::unique_ptr<ASTNode> then_block;
        std::unique_ptr<ASTNode> else_block;  // Can be BlockNode or another IfElseNode
    };
#endif