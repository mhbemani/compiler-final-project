#include "codegen.h"
#include <llvm/IR/Verifier.h>
#include <stdexcept>

using namespace llvm;

CodeGen::CodeGen() :
    context(std::make_unique<LLVMContext>()),
    module(std::make_unique<Module>("main", *context)),
    builder(std::make_unique<IRBuilder<>>(*context)) {

    // Create main function
    FunctionType* mainType = FunctionType::get(Type::getInt32Ty(*context), false);
    Function* mainFunc = Function::Create(mainType, Function::ExternalLinkage, "main", *module);
    
    // Create entry block
    BasicBlock* entry = BasicBlock::Create(*context, "entry", mainFunc);
    builder->SetInsertPoint(entry);
}

void CodeGen::generate(ProgramNode& ast) {
    for (auto& stmt : ast.statements) {
        generateStatement(stmt.get());
    }
    
    if (!builder->GetInsertBlock()->getTerminator()) {
        builder->CreateRet(ConstantInt::get(Type::getInt32Ty(*context), 0));
    }
    
    // Verify the module
    std::string error;
    raw_string_ostream os(error);
    if (verifyModule(*module, &os)) {
        throw std::runtime_error("Generated IR is invalid: " + error);
    }
}

void CodeGen::generateStatement(ASTNode* node) {
    if (auto multiVarDecl = dynamic_cast<MultiVarDeclNode*>(node)) {
        // Handle multiple variable declarations
        for (auto& decl : multiVarDecl->declarations) {
            generateVarDecl(decl.get());
        }
    } else if (auto varDecl = dynamic_cast<VarDeclNode*>(node)) {
        generateVarDecl(varDecl);
    } else if (auto assign = dynamic_cast<AssignNode*>(node)) {
        generateAssign(assign);
    }
}

void CodeGen::generateVarDecl(VarDeclNode* node) {
    if (symbols.find(node->name) != symbols.end()) {
        throw std::runtime_error("Redeclaration of variable: " + node->name);
    }
    
    // Create the appropriate type based on variable type
    Type* type;
    if (node->type == VarType::INT) {
        type = Type::getInt32Ty(*context);
    } else if (node->type == VarType::BOOL) {
        type = Type::getInt1Ty(*context); // Boolean type in LLVM is i1
    } else { // STRING
        type = PointerType::get(Type::getInt8Ty(*context), 0);
    }
    
    AllocaInst* alloca = builder->CreateAlloca(type, nullptr, node->name);
    symbols[node->name] = alloca;
    
    if (node->value) {
        Value* val = generateValue(node->value.get(), type);
        builder->CreateStore(val, alloca);
    }
}

void CodeGen::generateAssign(AssignNode* node) {
    auto it = symbols.find(node->name);
    if (it == symbols.end()) {
        throw std::runtime_error("Assignment to undeclared variable: " + node->name);
    }
    
    AllocaInst* alloca = it->second;
    Type* expectedType = alloca->getAllocatedType();
    Value* val = generateValue(node->value.get(), expectedType);
    
    builder->CreateStore(val, alloca);
}

llvm::Value* CodeGen::generateValue(ASTNode* node, llvm::Type* expectedType) {
    if (auto intLit = dynamic_cast<IntLiteral*>(node)) {
        if (!expectedType->isIntegerTy()) {
            throw std::runtime_error("Expected integer type");
        }
        return ConstantInt::get(Type::getInt32Ty(*context), intLit->value);
    } else if (auto strLit = dynamic_cast<StrLiteral*>(node)) {
        if (!expectedType->isPointerTy()) {
            throw std::runtime_error("Expected pointer type for string");
        }
        return builder->CreateGlobalStringPtr(strLit->value);
    } else if(auto boolLit = dynamic_cast<BoolLiteral*>(node)){
        if (!expectedType->isIntegerTy(1)) {
            throw std::runtime_error("Expected boolean type");
        }
        return ConstantInt::get(Type::getInt1Ty(*context), boolLit->value);
    }
    throw std::runtime_error("Unsupported value type in code generation");
}

void CodeGen::dump() const {
    module->print(llvm::outs(), nullptr);
}