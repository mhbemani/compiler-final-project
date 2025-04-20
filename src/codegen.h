#ifndef CODEGEN_H
#define CODEGEN_H

#include "ast.h"
#include <llvm/IR/LLVMContext.h>
#include <llvm/IR/Module.h>
#include <llvm/IR/IRBuilder.h>
#include <unordered_map>
#include <map>
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
    llvm::Function* printfFunc; 
    std::map<std::string, uint64_t> arraySizes;
    std::unordered_map<std::string, llvm::AllocaInst*> symbols;
    
    void generateStatement(ASTNode* node);
    void generateVarDecl(VarDeclNode* node);
    void generateAssign(AssignNode* node);
    void generateCompoundAssign(CompoundAssignNode* node);
    void generateIfElse(IfElseNode* node);
    void generateBlock(BlockNode* blockNode);
    void generatePrint(PrintNode* node);
    void generateLoop(LoopNode* node);
    void printArray(const std::vector<llvm::Value*>& elements);
    void printArrayVar(llvm::Value* arrayPtr, uint64_t size);
    llvm::Value* generatePow(llvm::Value* base, llvm::Value* exp);
    void generateTryCatch(TryCatchNode* node);
    void generateMatch(MatchNode* node);
    llvm::Value* generateValue(ASTNode* node, llvm::Type* expectedType);
};

#endif