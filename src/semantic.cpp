#include "semantic.h"
#include "ast.h"
#include <stdexcept>
#include <map>
#include <string>

SemanticAnalyzer::SemanticAnalyzer() {}

void SemanticAnalyzer::analyze(ProgramNode* program) {
    for (const auto& stmt : program->statements) {
        analyzeStatement(stmt.get());
    }
}

void SemanticAnalyzer::analyzeStatement(ASTNode* node) {
    if (auto* varDecl = dynamic_cast<VarDeclNode*>(node)) {
        analyzeVarDecl(varDecl);
    } else if (auto* assign = dynamic_cast<AssignNode*>(node)) {
        analyzeAssign(assign);
    } else if (auto* compoundAssign = dynamic_cast<CompoundAssignNode*>(node)) {
        analyzeCompoundAssign(compoundAssign);
    } else if (auto* binaryOp = dynamic_cast<BinaryOpNode*>(node)) {
        throw std::runtime_error("Standalone binary operation is not allowed as a statement");
    } else if (auto* unaryOp = dynamic_cast<UnaryOpNode*>(node)) {
        throw std::runtime_error("Standalone unary operation is not allowed as a statement");
    } else if (auto* print = dynamic_cast<PrintNode*>(node)) {
        getExpressionType(print->expr.get()); // Any type is valid for print
    } else if (auto* ifElse = dynamic_cast<IfElseNode*>(node)) {
        analyzeIfElse(ifElse);
    } else if (auto* loop = dynamic_cast<LoopNode*>(node)) {
        analyzeLoop(loop);
    } else if (auto* tryCatch = dynamic_cast<TryCatchNode*>(node)) {
        analyzeTryCatch(tryCatch);
    } else if (auto* match = dynamic_cast<MatchNode*>(node)) {
        analyzeMatch(match);
    } else if (auto* multiVarDecl = dynamic_cast<MultiVarDeclNode*>(node)) {
        for (const auto& decl : multiVarDecl->declarations) {
            analyzeVarDecl(decl.get());
        }
    } else if (auto* block = dynamic_cast<BlockNode*>(node)) {
        for (const auto& stmt : block->statements) {
            analyzeStatement(stmt.get());
        }
    } else {
        throw std::runtime_error("Unknown statement type in semantic analysis");
    }
}

void SemanticAnalyzer::analyzeVarDecl(VarDeclNode* node) {
    if (symbolTable.find(node->name) != symbolTable.end()) {
        throw std::runtime_error("Variable '" + node->name + "' already declared");
    }
    if (node->value) {
        VarType valueType = getExpressionType(node->value.get());
        if (valueType != node->type && !(node->type == VarType::FLOAT && valueType == VarType::INT)) {
            throw std::runtime_error("Type mismatch in declaration of '" + node->name + "': expected " + typeToString(node->type) + ", got " + typeToString(valueType));
        }
        if (node->type == VarType::ARRAY) {
            if (auto* arrayLit = dynamic_cast<ArrayLiteralNode*>(node->value.get())) {
                for (const auto& elem : arrayLit->elements) {
                    VarType elemType = getExpressionType(elem.get());
                    if (elemType != VarType::INT) { // Assuming array elements are INT; adjust if needed
                        throw std::runtime_error("Array elements must be integers, got " + typeToString(elemType));
                    }
                }
            } else {
                throw std::runtime_error("Array declaration requires array literal");
            }
        }
    }
    symbolTable[node->name] = node->type;
}

void SemanticAnalyzer::analyzeAssign(AssignNode* node) {
    auto it = symbolTable.find(node->name);
    if (it == symbolTable.end()) {
        throw std::runtime_error("Undefined variable '" + node->name + "' in assignment");
    }
    VarType varType = it->second;
    VarType valueType = getExpressionType(node->value.get());
    if (valueType != varType && !(varType == VarType::FLOAT && valueType == VarType::INT)) {
        throw std::runtime_error("Type mismatch in assignment to '" + node->name + "': expected " + typeToString(varType) + ", got " + typeToString(valueType));
    }
    if (varType == VarType::ARRAY) {
        if (auto* arrayLit = dynamic_cast<ArrayLiteralNode*>(node->value.get())) {
            for (const auto& elem : arrayLit->elements) {
                VarType elemType = getExpressionType(elem.get());
                if (elemType != VarType::INT) {
                    throw std::runtime_error("Array elements must be integers, got " + typeToString(elemType));
                }
            }
        } else {
            throw std::runtime_error("Array assignment requires array literal");
        }
    }
}

void SemanticAnalyzer::analyzeCompoundAssign(CompoundAssignNode* node) {
    auto it = symbolTable.find(node->name);
    if (it == symbolTable.end()) {
        throw std::runtime_error("Undefined variable '" + node->name + "' in compound assignment");
    }
    VarType varType = it->second;
    VarType valueType = getExpressionType(node->value.get());
    if (varType != VarType::INT && varType != VarType::FLOAT) {
        throw std::runtime_error("Compound assignment requires numeric variable, got " + typeToString(varType));
    }
    if (valueType != VarType::INT && valueType != VarType::FLOAT) {
        throw std::runtime_error("Compound assignment requires numeric value, got " + typeToString(valueType));
    }
}

VarType SemanticAnalyzer::getExpressionType(ASTNode* node) {
    if (auto* intLit = dynamic_cast<IntLiteral*>(node)) {
        return VarType::INT;
    } else if (auto* floatLit = dynamic_cast<FloatLiteral*>(node)) {
        return VarType::FLOAT;
    } else if (auto* strLit = dynamic_cast<StrLiteral*>(node)) {
        return VarType::STRING;
    } else if (auto* boolLit = dynamic_cast<BoolLiteral*>(node)) {
        return VarType::BOOL;
    } else if (auto* charLit = dynamic_cast<CharLiteral*>(node)) {
        return VarType::CHAR;
    } else if (auto* arrayLit = dynamic_cast<ArrayLiteralNode*>(node)) {
        if (arrayLit->elements.empty()) {
            return VarType::ARRAY; // Empty array
        }
        VarType elemType = getExpressionType(arrayLit->elements[0].get());
        for (const auto& elem : arrayLit->elements) {
            if (getExpressionType(elem.get()) != elemType) {
                throw std::runtime_error("Array elements must have consistent types");
            }
        }
        return VarType::ARRAY;
    } else if (auto* varRef = dynamic_cast<VarRefNode*>(node)) {
        auto it = symbolTable.find(varRef->name);
        if (it == symbolTable.end()) {
            throw std::runtime_error("Undefined variable '" + varRef->name + "'");
        }
        return it->second;
    } else if (auto* binaryOp = dynamic_cast<BinaryOpNode*>(node)) {
        return analyzeBinaryOp(binaryOp);
    } else if (auto* unaryOp = dynamic_cast<UnaryOpNode*>(node)) {
        return analyzeUnaryOp(unaryOp);
    } else if (auto* concat = dynamic_cast<ConcatNode*>(node)) {
        VarType leftType = getExpressionType(concat->left.get());
        VarType rightType = getExpressionType(concat->right.get());
        if ((leftType == VarType::STRING || leftType == VarType::CHAR || dynamic_cast<VarRefNode*>(concat->left.get())) &&
            (rightType == VarType::STRING || rightType == VarType::CHAR || dynamic_cast<VarRefNode*>(concat->right.get()))) {
            return VarType::STRING;
        }
        throw std::runtime_error("Concat requires string or char operands");
    } else if (auto* ternary = dynamic_cast<TernaryExprNode*>(node)) {
        VarType condType = getExpressionType(ternary->condition.get());
        if (condType != VarType::BOOL) {
            throw std::runtime_error("Ternary condition must be boolean");
        }
        VarType trueType = getExpressionType(ternary->trueBranch.get());
        VarType falseType = getExpressionType(ternary->falseBranch.get());
        if (trueType != falseType) {
            throw std::runtime_error("Ternary branches must have the same type");
        }
        return trueType;
    }
    throw std::runtime_error("Unknown expression type in semantic analysis");
}

VarType SemanticAnalyzer::analyzeBinaryOp(BinaryOpNode* node) {
    VarType leftType = getExpressionType(node->left.get());
    VarType rightType = node->right ? getExpressionType(node->right.get()) : leftType; // For ABS, right may be nullptr
    switch (node->op) {
        case BinaryOp::ADD:
        case BinaryOp::SUBTRACT:
        case BinaryOp::MULTIPLY:
        case BinaryOp::DIVIDE:
        case BinaryOp::MODULO:
        case BinaryOp::POW:
            if ((leftType != VarType::INT && leftType != VarType::FLOAT) ||
                (rightType != VarType::INT && rightType != VarType::FLOAT)) {
                throw std::runtime_error("Arithmetic operation requires numeric operands");
            }
            return (leftType == VarType::FLOAT || rightType == VarType::FLOAT) ? VarType::FLOAT : VarType::INT;
        case BinaryOp::EQUAL:
        case BinaryOp::NOT_EQUAL:
            if (leftType != rightType && !(leftType == VarType::FLOAT && rightType == VarType::INT) &&
                !(leftType == VarType::INT && rightType == VarType::FLOAT)) {
                throw std::runtime_error("Equality comparison requires matching types");
            }
            return VarType::BOOL;
        case BinaryOp::LESS:
        case BinaryOp::LESS_EQUAL:
        case BinaryOp::GREATER:
        case BinaryOp::GREATER_EQUAL:
            if ((leftType != VarType::INT && leftType != VarType::FLOAT) ||
                (rightType != VarType::INT && rightType != VarType::FLOAT)) {
                throw std::runtime_error("Comparison requires numeric operands");
            }
            return VarType::BOOL;
        case BinaryOp::AND:
        case BinaryOp::OR:
        case BinaryOp::XOR:
            if (leftType != VarType::BOOL || rightType != VarType::BOOL) {
                throw std::runtime_error("Logical operation requires boolean operands");
            }
            return VarType::BOOL;
        case BinaryOp::INDEX:
            if (leftType != VarType::ARRAY) {
                throw std::runtime_error("Indexing requires array operand");
            }
            if (rightType != VarType::INT) {
                throw std::runtime_error("Array index must be integer");
            }
            return VarType::INT; // Assuming array elements are INT; adjust if needed
        case BinaryOp::METHOD_CALL:
            if (leftType != VarType::ERROR || rightType != VarType::STRING) {
                throw std::runtime_error("Method call requires Error type and string method name");
            }
            return VarType::STRING; // e.g., e.toString()
        case BinaryOp::ABS:
            if (leftType != VarType::INT && leftType != VarType::FLOAT) {
                throw std::runtime_error("ABS requires numeric operand");
            }
            return leftType;
        case BinaryOp::MULTIPLY_ARRAY:
        case BinaryOp::ADD_ARRAY:
        case BinaryOp::SUBTRACT_ARRAY:
        case BinaryOp::DIVIDE_ARRAY:
            if (leftType != VarType::ARRAY || rightType != VarType::ARRAY) {
                throw std::runtime_error("Array operation requires array operands");
            }
            return VarType::ARRAY;
        default:
            throw std::runtime_error("Unknown binary operator in semantic analysis");
    }
}

VarType SemanticAnalyzer::analyzeUnaryOp(UnaryOpNode* node) {
    VarType operandType = getExpressionType(node->operand.get());
    switch (node->op) {
        case UnaryOp::INCREMENT:
        case UnaryOp::DECREMENT:
            if (operandType != VarType::INT && operandType != VarType::FLOAT) {
                throw std::runtime_error("Increment/decrement requires numeric operand");
            }
            return operandType;
        case UnaryOp::LENGTH:
        case UnaryOp::MIN:
        case UnaryOp::MAX:
            if (operandType != VarType::ARRAY) {
                throw std::runtime_error("Length/min/max requires array operand");
            }
            return (node->op == UnaryOp::LENGTH) ? VarType::INT : VarType::INT; // Adjust if elements are not INT
        default:
            throw std::runtime_error("Unknown unary operator in semantic analysis");
    }
}

void SemanticAnalyzer::analyzeIfElse(IfElseNode* node) {
    VarType condType = getExpressionType(node->condition.get());
    if (condType != VarType::BOOL) {
        throw std::runtime_error("If condition must be boolean");
    }
    analyzeStatement(node->then_block.get());
    if (node->else_block) {
        analyzeStatement(node->else_block.get());
    }
}

void SemanticAnalyzer::analyzeLoop(LoopNode* node) {
    if (node->varName.empty()) { // Traditional for loop
        if (node->init) {
            analyzeStatement(node->init.get());
        }
        if (node->condition) {
            VarType condType = getExpressionType(node->condition.get());
            if (condType != VarType::BOOL) {
                throw std::runtime_error("Loop condition must be boolean");
            }
        }
        if (node->update) {
            analyzeStatement(node->update.get());
        }
    } else { // Foreach loop
        VarType collType = getExpressionType(node->collection.get());
        if (collType != VarType::ARRAY) {
            throw std::runtime_error("Foreach collection must be array");
        }
        symbolTable[node->varName] = VarType::INT; // Assuming array elements are INT
    }
    analyzeStatement(node->body.get());
}

void SemanticAnalyzer::analyzeTryCatch(TryCatchNode* node) {
    analyzeStatement(node->tryBlock.get());
    symbolTable[node->errorVar] = VarType::ERROR;
    analyzeStatement(node->catchBlock.get());
    symbolTable.erase(node->errorVar); // Remove errorVar from scope
}

void SemanticAnalyzer::analyzeMatch(MatchNode* node) {
    VarType exprType = getExpressionType(node->expression.get());
    for (const auto& caseNode : node->cases) {
        if (caseNode->value) {
            VarType valueType = getExpressionType(caseNode->value.get());
            if (valueType != exprType) {
                throw std::runtime_error("Match case value type must match expression type");
            }
        }
        analyzeStatement(caseNode->body.get());
    }
}

std::string SemanticAnalyzer::typeToString(VarType type) {
    switch (type) {
        case VarType::INT: return "int";
        case VarType::STRING: return "string";
        case VarType::BOOL: return "bool";
        case VarType::FLOAT: return "float";
        case VarType::CHAR: return "char";
        case VarType::ARRAY: return "array";
        case VarType::ERROR: return "Error";
        default: return "unknown";
    }
}