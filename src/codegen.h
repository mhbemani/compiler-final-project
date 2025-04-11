#ifndef CODEGEN_H
#define CODEGEN_H

#include "ast.h"
#include <llvm/IR/LLVMContext.h>
#include <llvm/IR/Module.h>
#include <llvm/IR/IRBuilder.h>
#include <unordered_map>
#include <memory>

class CodeGen {
public:
    CodeGen();
    void generate(ProgramNode& ast);
    void dump() const;
    
private:
    std::unique_ptr<llvm::LLVMContext> context;
    std::unique_ptr<llvm::Module> module;
    std::unique_ptr<llvm::IRBuilder<>> builder;
    
    std::unordered_map<std::string, llvm::AllocaInst*> symbols;
    
    void generateStatement(ASTNode* node);
    void generateVarDecl(VarDeclNode* node);
    void generateAssign(AssignNode* node);
    void generateCompoundAssign(CompoundAssignNode* node);
    void generateIfElse(IfElseNode* node);
    void generateBlock(BlockNode* blockNode);
    llvm::Value* generateValue(ASTNode* node, llvm::Type* expectedType);
};

#endif