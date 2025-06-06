#include "optimizer.h"
#include <optional>
#include <memory>
#include <string>

void Optimizer::optimize(ProgramNode& program) {
    modifiedNodes.clear();
    for (size_t i = 0; i < program.statements.size(); ++i) {
        if (auto* loop = dynamic_cast<LoopNode*>(program.statements[i].get())) {
            if (loop->type == LoopType::For) {
                auto original = cloneNode(*loop);
                auto unrolled = unrollForLoop(*loop);
                if (unrolled) {
                    modifiedNodes.push_back({std::move(original), cloneNode(*unrolled)});
                    program.statements[i] = std::move(unrolled);
                }
            }
        } else if (auto* ifElse = dynamic_cast<IfElseNode*>(program.statements[i].get())) {
            auto result = evaluateConstantCondition(*ifElse->condition);
            if (result.has_value()) {
                auto original = cloneNode(*ifElse);
                auto newBlock = std::make_unique<BlockNode>();
                if (*result) {
                    newBlock->statements = std::move(dynamic_cast<BlockNode&>(*ifElse->then_block).statements);
                } else if (ifElse->else_block) {
                    newBlock->statements = std::move(dynamic_cast<BlockNode&>(*ifElse->else_block).statements);
                }
                modifiedNodes.push_back({std::move(original), cloneNode(*newBlock)});
                program.statements[i] = std::move(newBlock);
            } else {
                optimizeNode(*program.statements[i]);
            }
        } else {
            optimizeNode(*program.statements[i]);
        }
    }
}

void Optimizer::optimizeNode(ASTNode& node) {
    if (auto* block = dynamic_cast<BlockNode*>(&node)) {
        for (auto& stmt : block->statements) {
            optimizeNode(*stmt);
        }
    } else if (auto* match = dynamic_cast<MatchNode*>(&node)) {
        optimizeNode(*match->expression);
        for (auto& caseNode : match->cases) {
            optimizeNode(*caseNode->body);
            if (caseNode->value) {
                optimizeNode(*caseNode->value);
            }
        }
    } else if (auto* print = dynamic_cast<PrintNode*>(&node)) {
        optimizeNode(*print->expr);
    } else if (auto* assign = dynamic_cast<AssignNode*>(&node)) {
        optimizeNode(*assign->value);
    } else if (auto* compound = dynamic_cast<CompoundAssignNode*>(&node)) {
        optimizeNode(*compound->value);
    } else if (auto* unary = dynamic_cast<UnaryOpNode*>(&node)) {
        optimizeNode(*unary->operand);
    } else if (auto* binary = dynamic_cast<BinaryOpNode*>(&node)) {
        optimizeNode(*binary->left);
        if (binary->right) optimizeNode(*binary->right);
    } else if (auto* varDecl = dynamic_cast<VarDeclNode*>(&node)) {
        if (varDecl->value) {
            optimizeNode(*varDecl->value);
        }
    } else if (auto* concat = dynamic_cast<ConcatNode*>(&node)) {
        optimizeNode(*concat->left);
        optimizeNode(*concat->right);
    } else if (auto* tryCatch = dynamic_cast<TryCatchNode*>(&node)) {
        optimizeNode(*tryCatch->tryBlock);
        optimizeNode(*tryCatch->catchBlock);
    }
}

std::optional<bool> Optimizer::evaluateConstantCondition(const ASTNode& condition) const {
    if (auto* boolLit = dynamic_cast<const BoolLiteral*>(&condition)) {
        return boolLit->value;
    }
    if (auto* binary = dynamic_cast<const BinaryOpNode*>(&condition)) {
        if (binary->op == BinaryOp::AND || binary->op == BinaryOp::OR) {
            auto left = evaluateConstantCondition(*binary->left);
            auto right = evaluateConstantCondition(*binary->right);
            if (left.has_value() && right.has_value()) {
                if (binary->op == BinaryOp::AND) {
                    return *left && *right;
                } else {
                    return *left || *right;
                }
            }
        } else if (auto* leftLit = dynamic_cast<const IntLiteral*>(binary->left.get())) {
            if (auto* rightLit = dynamic_cast<const IntLiteral*>(binary->right.get())) {
                switch (binary->op) {
                    case BinaryOp::LESS: return leftLit->value < rightLit->value;
                    case BinaryOp::LESS_EQUAL: return leftLit->value <= rightLit->value;
                    case BinaryOp::GREATER: return leftLit->value > rightLit->value;
                    case BinaryOp::GREATER_EQUAL: return leftLit->value >= rightLit->value;
                    case BinaryOp::EQUAL: return leftLit->value == rightLit->value;
                    case BinaryOp::NOT_EQUAL: return leftLit->value != rightLit->value;
                    default: return std::nullopt;
                }
            }
        }
    }
    return std::nullopt;
}

std::string Optimizer::printNode(const ASTNode& node) const {
    if (auto* loop = dynamic_cast<const LoopNode*>(&node)) {
        if (loop->type == LoopType::For) {
            std::string result = "for (";
            if (auto* varDecl = dynamic_cast<VarDeclNode*>(loop->init.get())) {
                result += varDecl->name + " = " + printNode(*varDecl->value);
            } else if (auto* assign = dynamic_cast<AssignNode*>(loop->init.get())) {
                result += assign->name + " = " + printNode(*assign->value);
            }
            result += "; " + printNode(*loop->condition) + "; ";
            if (auto* unary = dynamic_cast<UnaryOpNode*>(loop->update.get())) {
                result += (unary->op == UnaryOp::INCREMENT) ? "++" : "--";
                result += dynamic_cast<VarRefNode*>(unary->operand.get())->name;
            }
            result += ") { ";
            result += printNode(*loop->body) + " }";
            return result;
        }
        return "[foreach loop]";
    } else if (auto* block = dynamic_cast<const BlockNode*>(&node)) {
        std::string result = "{ ";
        for (const auto& stmt : block->statements) {
            result += printNode(*stmt) + "; ";
        }
        result += "}";
        return result;
    } else if (auto* print = dynamic_cast<const PrintNode*>(&node)) {
        return "print(" + printNode(*print->expr) + ")";
    } else if (auto* intLit = dynamic_cast<const IntLiteral*>(&node)) {
        return std::to_string(intLit->value);
    } else if (auto* strLit = dynamic_cast<const StrLiteral*>(&node)) {
        return "\"" + strLit->value + "\"";
    } else if (auto* boolLit = dynamic_cast<const BoolLiteral*>(&node)) {
        return boolLit->value ? "true" : "false";
    } else if (auto* floatLit = dynamic_cast<const FloatLiteral*>(&node)) {
        return std::to_string(floatLit->value);
    } else if (auto* charLit = dynamic_cast<const CharLiteral*>(&node)) {
        return "'" + std::string(1, charLit->value) + "'";
    } else if (auto* varRef = dynamic_cast<const VarRefNode*>(&node)) {
        return varRef->name;
    } else if (auto* assign = dynamic_cast<const AssignNode*>(&node)) {
        return assign->name + " = " + printNode(*assign->value);
    } else if (auto* binary = dynamic_cast<const BinaryOpNode*>(&node)) {
        std::string opStr;
        switch (binary->op) {
            case BinaryOp::ADD: opStr = "+"; break;
            case BinaryOp::SUBTRACT: opStr = "-"; break;
            case BinaryOp::MULTIPLY: opStr = "*"; break;
            case BinaryOp::DIVIDE: opStr = "/"; break;
            case BinaryOp::EQUAL: opStr = "=="; break;
            case BinaryOp::NOT_EQUAL: opStr = "!="; break;
            case BinaryOp::LESS: opStr = "<"; break;
            case BinaryOp::LESS_EQUAL: opStr = "<="; break;
            case BinaryOp::GREATER: opStr = ">"; break;
            case BinaryOp::GREATER_EQUAL: opStr = ">="; break;
            case BinaryOp::AND: opStr = "&&"; break;
            case BinaryOp::OR: opStr = "||"; break;
            case BinaryOp::INDEX: opStr = "[]"; break;
            case BinaryOp::CONCAT: opStr = "+"; break;
            default: opStr = "?"; break;
        }
        return printNode(*binary->left) + " " + opStr + " " + (binary->right ? printNode(*binary->right) : "");
    } else if (auto* varDecl = dynamic_cast<const VarDeclNode*>(&node)) {
        std::string typeStr;
        switch (varDecl->type) {
            case VarType::INT: typeStr = "int"; break;
            case VarType::STRING: typeStr = "string"; break;
            case VarType::BOOL: typeStr = "bool"; break;
            case VarType::FLOAT: typeStr = "float"; break;
            case VarType::CHAR: typeStr = "char"; break;
            case VarType::ARRAY: typeStr = "array"; break;
            default: typeStr = "unknown"; break;
        }
        return typeStr + " " + varDecl->name + " = " + (varDecl->value ? printNode(*varDecl->value) : "");
    } else if (auto* ternary = dynamic_cast<const TernaryExprNode*>(&node)) {
        return printNode(*ternary->condition) + " ? " + printNode(*ternary->trueBranch) + " : " + printNode(*ternary->falseBranch);
    }
    return "[unknown]";
}

void Optimizer::printModifiedNodes() const {
    if (modifiedNodes.empty()) {
        return;
    }
    for (size_t i = 0; i < modifiedNodes.size(); ++i) {
        std::string original = printNode(*modifiedNodes[i].original);
        std::string modified = printNode(*modifiedNodes[i].modified);
    }
}

std::unique_ptr<ASTNode> Optimizer::unrollForLoop(LoopNode& loop) {
    auto bounds = getLoopBounds(loop);
    if (!bounds) {
        return nullptr;
    }

    auto [start, end, step] = *bounds;
    auto* cond = dynamic_cast<BinaryOpNode*>(loop.condition.get());
    if (!cond) {
        return nullptr;
    }
    int iterations = computeIterations(start, end, step, cond->op);
    if (iterations < 0) {
        return nullptr;
    }

    std::string loopVar = getLoopVariable(loop);
    if (loopVar.empty()) {
        return nullptr;
    }

    auto* body = dynamic_cast<BlockNode*>(loop.body.get());
    if (!body) {
        return nullptr;
    }
    auto unrolled = std::make_unique<BlockNode>();
    for (int j = 0; j < iterations; ++j) {
        int value = (step > 0) ? start + j * step : start - j * (-step);
        for (const auto& stmt : body->statements) {
            auto clonedStmt = cloneNode(*stmt);
            substituteVariable(*clonedStmt, loopVar, value);
            unrolled->statements.push_back(std::move(clonedStmt));
        }
    }
    return unrolled;
}

std::optional<std::tuple<int, int, int>> Optimizer::getLoopBounds(const LoopNode& loop) {
    int start = 0;
    std::string varName;
    if (auto* varDecl = dynamic_cast<VarDeclNode*>(loop.init.get())) {
        if (auto* initLit = dynamic_cast<IntLiteral*>(varDecl->value.get())) {
            start = initLit->value;
            varName = varDecl->name;
        } else {
            return std::nullopt;
        }
    } else if (auto* assign = dynamic_cast<AssignNode*>(loop.init.get())) {
        if (auto* initLit = dynamic_cast<IntLiteral*>(assign->value.get())) {
            start = initLit->value;
            varName = assign->name;
        } else {
            return std::nullopt;
        }
    } else {
        return std::nullopt;
    }

    int end = 0;
    auto* cond = dynamic_cast<BinaryOpNode*>(loop.condition.get());
    if (!cond || !(cond->op == BinaryOp::LESS || cond->op == BinaryOp::LESS_EQUAL ||
                   cond->op == BinaryOp::GREATER || cond->op == BinaryOp::GREATER_EQUAL)) {
        return std::nullopt;
    }
    if (auto* leftVar = dynamic_cast<VarRefNode*>(cond->left.get())) {
        if (leftVar->name != varName) {
            return std::nullopt;
        }
        if (auto* rightLit = dynamic_cast<IntLiteral*>(cond->right.get())) {
            end = rightLit->value;
        } else {
            return std::nullopt;
        }
    } else {
        return std::nullopt;
    }

    int step = 0;
    if (auto* unary = dynamic_cast<UnaryOpNode*>(loop.update.get())) {
        if (unary->op == UnaryOp::INCREMENT && dynamic_cast<VarRefNode*>(unary->operand.get())->name == varName) {
            step = 1;
        } else if (unary->op == UnaryOp::DECREMENT && dynamic_cast<VarRefNode*>(unary->operand.get())->name == varName) {
            step = -1;
        } else {
            return std::nullopt;
        }
    } else if (auto* assign = dynamic_cast<AssignNode*>(loop.update.get())) {
        if (assign->name != varName) {
            return std::nullopt;
        }
        if (auto* binOp = dynamic_cast<BinaryOpNode*>(assign->value.get())) {
            if (binOp->op == BinaryOp::ADD && dynamic_cast<VarRefNode*>(binOp->left.get())->name == varName) {
                if (auto* stepLit = dynamic_cast<IntLiteral*>(binOp->right.get())) {
                    step = stepLit->value;
                }
            } else if (binOp->op == BinaryOp::SUBTRACT && dynamic_cast<VarRefNode*>(binOp->left.get())->name == varName) {
                if (auto* stepLit = dynamic_cast<IntLiteral*>(binOp->right.get())) {
                    step = -stepLit->value;
                }
            }
        }
    } else {
        return std::nullopt;
    }

    return std::make_tuple(start, end, step);
}

int Optimizer::computeIterations(int start, int end, int step, BinaryOp op) {
    if (step == 0) return 0;
    if (step > 0) {
        if (op == BinaryOp::LESS) {
            return (end - start) / step;
        } else if (op == BinaryOp::LESS_EQUAL) {
            return (end - start + step) / step;
        }
    } else if (step < 0) {
        if (op == BinaryOp::GREATER) {
            return (start - end) / (-step);
        } else if (op == BinaryOp::GREATER_EQUAL) {
            return (start - end - step) / (-step);
        }
    }
    return 0;
}

std::string Optimizer::getLoopVariable(const LoopNode& loop) {
    if (auto* varDecl = dynamic_cast<VarDeclNode*>(loop.init.get())) {
        return varDecl->name;
    } else if (auto* assign = dynamic_cast<AssignNode*>(loop.init.get())) {
        return assign->name;
    }
    return "";
}

std::unique_ptr<ASTNode> Optimizer::cloneNode(const ASTNode& node) {
    if (auto* block = dynamic_cast<const BlockNode*>(&node)) {
        auto newBlock = std::make_unique<BlockNode>();
        for (const auto& stmt : block->statements) {
            newBlock->statements.push_back(cloneNode(*stmt));
        }
        return newBlock;
    } else if (auto* loop = dynamic_cast<const LoopNode*>(&node)) {
        if (loop->type == LoopType::For) {
            return std::make_unique<LoopNode>(
                cloneNode(*loop->init), cloneNode(*loop->condition),
                cloneNode(*loop->update), cloneNode(*loop->body));
        }
        return nullptr;
    } else if (auto* print = dynamic_cast<const PrintNode*>(&node)) {
        return std::make_unique<PrintNode>(cloneNode(*print->expr));
    } else if (auto* varRef = dynamic_cast<const VarRefNode*>(&node)) {
        return std::make_unique<VarRefNode>(varRef->name);
    } else if (auto* intLit = dynamic_cast<const IntLiteral*>(&node)) {
        return std::make_unique<IntLiteral>(intLit->value);
    } else if (auto* strLit = dynamic_cast<const StrLiteral*>(&node)) {
        return std::make_unique<StrLiteral>(strLit->value);
    } else if (auto* boolLit = dynamic_cast<const BoolLiteral*>(&node)) {
        return std::make_unique<BoolLiteral>(boolLit->value);
    } else if (auto* floatLit = dynamic_cast<const FloatLiteral*>(&node)) {
        return std::make_unique<FloatLiteral>(floatLit->value);
    } else if (auto* charLit = dynamic_cast<const CharLiteral*>(&node)) {
        return std::make_unique<CharLiteral>(charLit->value);
    } else if (auto* binary = dynamic_cast<const BinaryOpNode*>(&node)) {
        return std::make_unique<BinaryOpNode>(binary->op, cloneNode(*binary->left),
                                             binary->right ? cloneNode(*binary->right) : nullptr);
    } else if (auto* unary = dynamic_cast<const UnaryOpNode*>(&node)) {
        return std::make_unique<UnaryOpNode>(unary->op, cloneNode(*unary->operand));
    } else if (auto* assign = dynamic_cast<const AssignNode*>(&node)) {
        return std::make_unique<AssignNode>(assign->name, cloneNode(*assign->value));
    } else if (auto* arrayLit = dynamic_cast<const ArrayLiteralNode*>(&node)) {
        std::vector<std::unique_ptr<ASTNode>> elements;
        for (const auto& elem : arrayLit->elements) {
            elements.push_back(cloneNode(*elem));
        }
        return std::make_unique<ArrayLiteralNode>(std::move(elements));
    } else if (auto* concat = dynamic_cast<const ConcatNode*>(&node)) {
        return std::make_unique<ConcatNode>(cloneNode(*concat->left), cloneNode(*concat->right));
    } else if (auto* varDecl = dynamic_cast<const VarDeclNode*>(&node)) {
        return std::make_unique<VarDeclNode>(varDecl->type, varDecl->name,
                                             varDecl->value ? cloneNode(*varDecl->value) : nullptr);
    } else if (auto* ternary = dynamic_cast<const TernaryExprNode*>(&node)) {
        return std::make_unique<TernaryExprNode>(cloneNode(*ternary->condition),
                                                 cloneNode(*ternary->trueBranch), cloneNode(*ternary->falseBranch));
    }
    return nullptr;
}

void Optimizer::substituteVariable(ASTNode& node, const std::string& var, int value) {
    if (auto* block = dynamic_cast<BlockNode*>(&node)) {
        for (auto& stmt : block->statements) {
            substituteVariable(*stmt, var, value);
        }
    } else if (auto* print = dynamic_cast<PrintNode*>(&node)) {
        if (auto* exprVar = dynamic_cast<VarRefNode*>(print->expr.get())) {
            if (exprVar->name == var) {
                print->expr = std::make_unique<IntLiteral>(value);
            }
        } else {
            substituteVariable(*print->expr, var, value);
        }
    } else if (auto* binary = dynamic_cast<BinaryOpNode*>(&node)) {
        if (auto* leftVar = dynamic_cast<VarRefNode*>(binary->left.get())) {
            if (leftVar->name == var) {
                binary->left = std::make_unique<IntLiteral>(value);
            }
        } else {
            substituteVariable(*binary->left, var, value);
        }
        if (binary->right) {
            if (auto* rightVar = dynamic_cast<VarRefNode*>(binary->right.get())) {
                if (rightVar->name == var) {
                    binary->right = std::make_unique<IntLiteral>(value);
                }
            } else {
                substituteVariable(*binary->right, var, value);
            }
        }
    } else if (auto* unary = dynamic_cast<UnaryOpNode*>(&node)) {
        if (auto* operandVar = dynamic_cast<VarRefNode*>(unary->operand.get())) {
            if (operandVar->name == var) {
                unary->operand = std::make_unique<IntLiteral>(value);
            }
        } else {
            substituteVariable(*unary->operand, var, value);
        }
    } else if (auto* assign = dynamic_cast<AssignNode*>(&node)) {
        if (auto* valueVar = dynamic_cast<VarRefNode*>(assign->value.get())) {
            if (valueVar->name == var) {
                assign->value = std::make_unique<IntLiteral>(value);
            }
        } else {
            substituteVariable(*assign->value, var, value);
        }
    } else if (auto* arrayLit = dynamic_cast<ArrayLiteralNode*>(&node)) {
        for (auto& elem : arrayLit->elements) {
            substituteVariable(*elem, var, value);
        }
    } else if (auto* concat = dynamic_cast<ConcatNode*>(&node)) {
        if (auto* leftVar = dynamic_cast<VarRefNode*>(concat->left.get())) {
            if (leftVar->name == var) {
                concat->left = std::make_unique<IntLiteral>(value);
            }
        } else {
            substituteVariable(*concat->left, var, value);
        }
        if (auto* rightVar = dynamic_cast<VarRefNode*>(concat->right.get())) {
            if (rightVar->name == var) {
                concat->right = std::make_unique<IntLiteral>(value);
            }
        } else {
            substituteVariable(*concat->right, var, value);
        }
    } else if (auto* varDecl = dynamic_cast<VarDeclNode*>(&node)) {
        if (varDecl->value) {
            if (auto* valueVar = dynamic_cast<VarRefNode*>(varDecl->value.get())) {
                if (valueVar->name == var) {
                    varDecl->value = std::make_unique<IntLiteral>(value);
                }
            } else {
                substituteVariable(*varDecl->value, var, value);
            }
        }
    }
}