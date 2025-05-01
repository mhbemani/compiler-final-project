#ifndef SEMANTIC_H
#define SEMANTIC_H
#include "ast.h"
#include <map>
#include <string>

class SemanticAnalyzer {
private:
    std::map<std::string, VarType> symbolTable;
    void analyzeStatement(ASTNode* node);
    void analyzeVarDecl(VarDeclNode* node);
    void analyzeAssign(AssignNode* node);
    void analyzeCompoundAssign(CompoundAssignNode* node);
    VarType getExpressionType(ASTNode* node);
    VarType analyzeBinaryOp(BinaryOpNode* node);
    VarType analyzeUnaryOp(UnaryOpNode* node);
    void analyzeIfElse(IfElseNode* node);
    void analyzeLoop(LoopNode* node);
    void analyzeTryCatch(TryCatchNode* node);
    void analyzeMatch(MatchNode* node);
    std::string typeToString(VarType type);

public:
    SemanticAnalyzer();
    void analyze(ProgramNode* program);
};

#endif