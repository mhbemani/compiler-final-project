#ifndef AST_H
#define AST_H

#include <memory>
#include <vector>
#include <string>

enum class VarType { INT, STRING, BOOL, FLOAT, CHAR, NEUTRAL, ARRAY, ERROR };
enum class BinaryOp { ADD, SUBTRACT, MULTIPLY, DIVIDE, EQUAL, ABS, POW,
     NOT_EQUAL, LESS, LESS_EQUAL, GREATER, GREATER_EQUAL, AND, OR , XOR, MODULO, 
     INDEX, MULTIPLY_ARRAY, ADD_ARRAY, SUBTRACT_ARRAY, DIVIDE_ARRAY, CONCAT, METHOD_CALL };
enum class LoopType { For, Foreach };
enum class UnaryOp { LENGTH, MIN, MAX, INCREMENT ,DECREMENT, NEGATE };
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

class PrintNode : public ASTNode {
    public:
        std::unique_ptr<ASTNode> expr;  // Expression to print (literal, var, or operation)
        PrintNode(std::unique_ptr<ASTNode> expr) : expr(std::move(expr)) {}
};

class LoopNode : public ASTNode {
    public:
        LoopType type;
        // For 'for' loop
        std::unique_ptr<ASTNode> init;      // Optional: VarDeclNode or AssignNode
        std::unique_ptr<ASTNode> condition; // Optional: Expression
        std::unique_ptr<ASTNode> update;    // Optional: Expression
        // For 'foreach' loop
        std::string varName;                // Optional: Loop variable (e.g., x)
        std::unique_ptr<ASTNode> collection; // Optional: Collection expression (e.g., nums, multiply(arr, arr))
        // Common
        std::unique_ptr<ASTNode> body;      // Required: BlockNode
    
        // Constructor for 'for'
        LoopNode(std::unique_ptr<ASTNode> init, std::unique_ptr<ASTNode> condition,
                 std::unique_ptr<ASTNode> update, std::unique_ptr<ASTNode> body)
            : type(LoopType::For), init(std::move(init)), condition(std::move(condition)),
              update(std::move(update)), body(std::move(body)) {}
    
        // Constructor for 'foreach'
        LoopNode(std::string varName, std::unique_ptr<ASTNode> collection, std::unique_ptr<ASTNode> body)
            : type(LoopType::Foreach), varName(std::move(varName)), 
              collection(std::move(collection)), body(std::move(body)) {}
    };

struct ConcatNode : ASTNode {
    std::unique_ptr<ASTNode> left;
    std::unique_ptr<ASTNode> right;
    ConcatNode(std::unique_ptr<ASTNode> l, std::unique_ptr<ASTNode> r)
        : left(std::move(l)), right(std::move(r)) {}
};

class ArrayLiteralNode : public ASTNode {
    public:
        std::vector<std::unique_ptr<ASTNode>> elements;
        ArrayLiteralNode(std::vector<std::unique_ptr<ASTNode>> elements)
            : elements(std::move(elements)) {}
};

class UnaryOpNode : public ASTNode { // NEW (assuming it wasn't present)
    public:
        UnaryOp op;
        std::unique_ptr<ASTNode> operand;
        UnaryOpNode(UnaryOp op, std::unique_ptr<ASTNode> operand)
            : op(op), operand(std::move(operand)) {}
};

class TryCatchNode : public ASTNode {
    public:
        std::unique_ptr<BlockNode> tryBlock;
        std::unique_ptr<BlockNode> catchBlock;
        std::string errorVar; // e in catch (Error e)
        TryCatchNode(std::unique_ptr<BlockNode> tryBlock, 
                     std::unique_ptr<BlockNode> catchBlock,
                     std::string errorVar)
            : tryBlock(std::move(tryBlock)), catchBlock(std::move(catchBlock)), 
              errorVar(std::move(errorVar)) {}
    };

struct TernaryExprNode : ASTNode {
    std::unique_ptr<ASTNode> condition;    // e.g., z > 5
    std::unique_ptr<ASTNode> trueBranch;   // e.g., y
    std::unique_ptr<ASTNode> falseBranch;  // e.g., w
    TernaryExprNode(std::unique_ptr<ASTNode> cond, std::unique_ptr<ASTNode> trueB, std::unique_ptr<ASTNode> falseB)
        : condition(std::move(cond)), trueBranch(std::move(trueB)), falseBranch(std::move(falseB)) {}
};

// Match case node (e.g., 0 -> stmt or _ -> stmt)
struct MatchCaseNode : ASTNode {
    std::unique_ptr<ASTNode> value; // Case value (e.g., 0, 1, or nullptr for _)
    std::unique_ptr<ASTNode> body;  // Body (StatementNode or BlockNode)
    MatchCaseNode(std::unique_ptr<ASTNode> val, std::unique_ptr<ASTNode> b)
        : value(std::move(val)), body(std::move(b)) {}
};

// Match statement node (e.g., match x { 0 -> stmt, _ -> stmt })
struct MatchNode : ASTNode {
    std::unique_ptr<ASTNode> expression; // Match expression (e.g., x)
    std::vector<std::unique_ptr<MatchCaseNode>> cases; // List of cases
    MatchNode(std::unique_ptr<ASTNode> expr, std::vector<std::unique_ptr<MatchCaseNode>> c)
        : expression(std::move(expr)), cases(std::move(c)) {}
};

#endif