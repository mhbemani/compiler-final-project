#ifndef OPTIMIZER_H
#define OPTIMIZER_H

#include "ast.h"
#include <vector>
#include <memory>

class Optimizer {
public:
    void optimize(ProgramNode& program);
    void printModifiedNodes() const;

// private:
    struct ModifiedNode {
        std::unique_ptr<ASTNode> original;
        std::unique_ptr<ASTNode> modified;
    };
    std::vector<ModifiedNode> modifiedNodes;
    std::optional<bool> evaluateConstantCondition(const ASTNode& condition) const; ////
    void optimizeNode(ASTNode& node);
    std::string printNode(const ASTNode& node) const;
    std::unique_ptr<ASTNode> unrollForLoop(LoopNode& loop);
    std::optional<std::tuple<int, int, int>> getLoopBounds(const LoopNode& loop);
    int computeIterations(int start, int end, int step, BinaryOp op);
    std::string getLoopVariable(const LoopNode& loop);
    std::unique_ptr<ASTNode> cloneNode(const ASTNode& node);
    void substituteVariable(ASTNode& node, const std::string& var, int value);
};

#endif