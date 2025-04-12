#include "codegen.h"
#include <llvm/IR/Verifier.h>
#include <stdexcept>

using namespace llvm;

CodeGen::CodeGen() :
    context(std::make_unique<LLVMContext>()),
    module(std::make_unique<Module>("main", *context)),
    builder(std::make_unique<IRBuilder<>>(*context)),
    arraySizes() {

    // Create main function
    FunctionType* mainType = FunctionType::get(Type::getInt32Ty(*context), false);
    Function* mainFunc = Function::Create(mainType, Function::ExternalLinkage, "main", *module);
    FunctionType* printfType = FunctionType::get(Type::getInt32Ty(*context),
                                                {PointerType::get(Type::getInt8Ty(*context), 0)}, true);
    printfFunc = Function::Create(printfType, Function::ExternalLinkage, "printf", module.get());

    // Declare string functions for concat
    Type* int8PtrTy = PointerType::get(Type::getInt8Ty(*context), 0);
    Type* int32Ty = Type::getInt32Ty(*context);
    // strlen: i32 (i8*)
    FunctionType* strlenType = FunctionType::get(int32Ty, {int8PtrTy}, false);
    Function::Create(strlenType, Function::ExternalLinkage, "strlen", module.get());
    // malloc: i8* (i32)
    FunctionType* mallocType = FunctionType::get(int8PtrTy, {int32Ty}, false);
    Function::Create(mallocType, Function::ExternalLinkage, "malloc", module.get());
    // memcpy: i8* (i8*, i8*, i32)
    FunctionType* memcpyType = FunctionType::get(int8PtrTy, {int8PtrTy, int8PtrTy, int32Ty}, false);
    Function::Create(memcpyType, Function::ExternalLinkage, "memcpy", module.get());

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
    } else if (auto compound = dynamic_cast<CompoundAssignNode*>(node)) { // new
        generateCompoundAssign(compound);
    } else if (auto ifElseNode = dynamic_cast<IfElseNode*>(node)) {
        // Handle if-else statement
        generateIfElse(ifElseNode); // Call the updated generateIfElse
    } else if (auto print = dynamic_cast<PrintNode*>(node)) {  // New case
        generatePrint(print);
    } else if (auto loop = dynamic_cast<LoopNode*>(node)) {
        generateLoop(loop);
    } else if (auto block = dynamic_cast<BlockNode*>(node)) { // New case
        generateBlock(block);
    } else if (auto concat = dynamic_cast<ConcatNode*>(node)) {
        generateValue(concat, nullptr);
    }else {
        throw std::runtime_error("Unknown statement type");
    }
}

void CodeGen::generateVarDecl(VarDeclNode* node) {
    // if (symbols.find(node->name) != symbols.end()) {
    //     throw std::runtime_error("Redeclaration of variable: " + node->name);
    // }
    
    // Create the appropriate type based on variable type
    Type* type;
    switch (node->type) {
        case VarType::INT: type = Type::getInt32Ty(*context); break;
        case VarType::BOOL: type = Type::getInt1Ty(*context); break;
        case VarType::FLOAT: type = Type::getFloatTy(*context); break;
        case VarType::CHAR: type = Type::getInt8Ty(*context); break;
        case VarType::STRING: type = PointerType::get(Type::getInt8Ty(*context), 0); break;
        case VarType::ARRAY: type = PointerType::get(Type::getInt32Ty(*context), 0); break; //////
        default: throw std::runtime_error("Unknown variable type");
    }
    
    AllocaInst* alloca = builder->CreateAlloca(type, nullptr, node->name);
    symbols[node->name] = alloca;
    
    if (node->value) { // CHANGED: initializer -> value
        Value* val = generateValue(node->value.get(), type);
        builder->CreateStore(val, alloca);
        if (node->type == VarType::ARRAY && dynamic_cast<ArrayLiteralNode*>(node->value.get())) {
            auto* arrLit = dynamic_cast<ArrayLiteralNode*>(node->value.get());
            arraySizes[node->name] = arrLit->elements.size();
        }
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

void CodeGen::generateCompoundAssign(CompoundAssignNode* node) {
    auto it = symbols.find(node->name);
    if (it == symbols.end()) {
        throw std::runtime_error("Compound assignment to undeclared variable: " + node->name);
    }

    AllocaInst* alloca = it->second;
    Type* type = alloca->getAllocatedType();

    // Load current value
    Value* current = builder->CreateLoad(type, alloca);

    // Evaluate right-hand side
    Value* rhs = generateValue(node->value.get(), type);

    // Perform the operation
    Value* result = nullptr;
    switch (node->op) {
        case BinaryOp::ADD:
            result = type->isFloatTy() ? builder->CreateFAdd(current, rhs)
                                       : builder->CreateAdd(current, rhs);
            break;
        case BinaryOp::SUBTRACT:
            result = type->isFloatTy() ? builder->CreateFSub(current, rhs)
                                       : builder->CreateSub(current, rhs);
            break;
        case BinaryOp::MULTIPLY:
            result = type->isFloatTy() ? builder->CreateFMul(current, rhs)
                                       : builder->CreateMul(current, rhs);
            break;
        case BinaryOp::DIVIDE:
            result = type->isFloatTy() ? builder->CreateFDiv(current, rhs)
                                       : builder->CreateSDiv(current, rhs);  // Signed integer division
            break;
        default:
            throw std::runtime_error("Unsupported compound assignment operator");
    }

    // Store result back
    builder->CreateStore(result, alloca);
}

void CodeGen::generateIfElse(IfElseNode* node) {
    Value* conditionValue = generateValue(node->condition.get(), Type::getInt1Ty(*context));
    Function* parentFunc = builder->GetInsertBlock()->getParent();
    BasicBlock* thenBlock = BasicBlock::Create(*context, "then", parentFunc);
    BasicBlock* elseBlock = node->hasElseBlock() ? BasicBlock::Create(*context, "else", parentFunc) : nullptr;
    BasicBlock* afterIfElseBlock = BasicBlock::Create(*context, "after_if_else", parentFunc);

    builder->CreateCondBr(conditionValue, thenBlock, elseBlock ? elseBlock : afterIfElseBlock);

    builder->SetInsertPoint(thenBlock);
    if (auto* block = dynamic_cast<BlockNode*>(node->then_block.get())) {
        generateBlock(block);
    } else {
        throw std::runtime_error("Expected BlockNode for then_block in IfElseNode");
    }
    if (!builder->GetInsertBlock()->getTerminator()) {
        builder->CreateBr(afterIfElseBlock);
    }

    if (elseBlock) {
        builder->SetInsertPoint(elseBlock);
        if (node->isElseIf()) {
            generateIfElse(dynamic_cast<IfElseNode*>(node->else_block.get()));
            // Ensure the recursive call's block ends properly; rely on its after_if_else
        } else if (auto* block = dynamic_cast<BlockNode*>(node->else_block.get())) {
            generateBlock(block);
            if (!builder->GetInsertBlock()->getTerminator()) {
                builder->CreateBr(afterIfElseBlock);
            }
        } else {
            throw std::runtime_error("Expected BlockNode or IfElseNode for else_block");
        }
        // Add a terminator if the else block itself needs one
        if (!builder->GetInsertBlock()->getTerminator()) {
            builder->CreateBr(afterIfElseBlock);
        }
    }

    builder->SetInsertPoint(afterIfElseBlock);
}

void CodeGen::generatePrint(PrintNode* node) {
    Value* value = nullptr;
    Type* valueType = nullptr;

    if (auto* arrLit = dynamic_cast<ArrayLiteralNode*>(node->expr.get())) { // NEW: Array literal
        // Print array as [a, b, c]
        std::vector<Value*> elements;
        for (const auto& elem : arrLit->elements) {
            elements.push_back(generateValue(elem.get(), Type::getInt32Ty(*context)));
        }
        printArray(elements);
        return;
    } else if (auto* varRef = dynamic_cast<VarRefNode*>(node->expr.get())) {
        auto it = symbols.find(varRef->name);
        if (it == symbols.end()) {
            throw std::runtime_error("Undefined variable: " + varRef->name);
        }
        valueType = it->second->getAllocatedType();
        if (valueType == PointerType::get(Type::getInt32Ty(*context), 0)) { // NEW: Array variable
            // Load array pointer (simplified: assumes fixed size for printing)
            value = builder->CreateLoad(valueType, it->second);
            auto sizeIt = arraySizes.find(varRef->name);
            uint64_t size = sizeIt != arraySizes.end() ? sizeIt->second : 5; // Fallback
            printArrayVar(value, size);
            return;
        }
        value = generateValue(node->expr.get(), valueType);
    }
    else if (dynamic_cast<IntLiteral*>(node->expr.get())) {
        value = generateValue(node->expr.get(), Type::getInt32Ty(*context));
        valueType = Type::getInt32Ty(*context);
    }
    else if (auto varRef = dynamic_cast<VarRefNode*>(node->expr.get())) {
        auto it = symbols.find(varRef->name);
        if (it == symbols.end()) {
            throw std::runtime_error("Undefined variable: " + varRef->name);
        }
        value = generateValue(node->expr.get(), it->second->getAllocatedType());
        valueType = it->second->getAllocatedType();
    }
    else if (dynamic_cast<BinaryOpNode*>(node->expr.get())) {
        if (auto binOp = dynamic_cast<BinaryOpNode*>(node->expr.get())) {
            if (binOp->op == BinaryOp::EQUAL || binOp->op == BinaryOp::LESS_EQUAL ||
                binOp->op == BinaryOp::NOT_EQUAL || binOp->op == BinaryOp::GREATER ||
                binOp->op == BinaryOp::GREATER_EQUAL || binOp->op == BinaryOp::LESS ||
                binOp->op == BinaryOp::AND || binOp->op == BinaryOp::OR) {
                value = generateValue(node->expr.get(), Type::getInt1Ty(*context));
                valueType = Type::getInt1Ty(*context);
            } else if (binOp->op == BinaryOp::INDEX) {
                value = generateValue(node->expr.get(), Type::getInt32Ty(*context));
                valueType = Type::getInt32Ty(*context);
            } else if (binOp->op == BinaryOp::MULTIPLY_ARRAY || binOp->op == BinaryOp::ADD_ARRAY ||
                       binOp->op == BinaryOp::SUBTRACT_ARRAY || binOp->op == BinaryOp::DIVIDE_ARRAY) {
                value = generateValue(node->expr.get(), PointerType::get(Type::getInt32Ty(*context), 0));
                valueType = PointerType::get(Type::getInt32Ty(*context), 0);
                auto* varRef = dynamic_cast<VarRefNode*>(binOp->left.get());
                uint64_t size = 5;
                if (varRef) {
                    auto sizeIt = arraySizes.find(varRef->name);
                    if (sizeIt != arraySizes.end()) size = sizeIt->second;
                }
                printArrayVar(value, size);
                return;
            } else {
                value = generateValue(node->expr.get(), Type::getInt32Ty(*context));
                valueType = Type::getInt32Ty(*context);
            }
        }
    }
    else if (auto strLit = dynamic_cast<StrLiteral*>(node->expr.get())) {
        value = generateValue(node->expr.get(), PointerType::get(Type::getInt8Ty(*context), 0));
        valueType = PointerType::get(Type::getInt8Ty(*context), 0);
    }
    else if (auto concat = dynamic_cast<ConcatNode*>(node->expr.get())) {
        value = generateValue(node->expr.get(), PointerType::get(Type::getInt8Ty(*context), 0));
        valueType = PointerType::get(Type::getInt8Ty(*context), 0);
    }
    else if (auto unaryOp = dynamic_cast<UnaryOpNode*>(node->expr.get())) {
        value = generateValue(node->expr.get(), Type::getInt32Ty(*context));
        valueType = Type::getInt32Ty(*context);
    }
    else {
        throw std::runtime_error("Unsupported type in print()");
    }

    Type* int32Ty = Type::getInt32Ty(*context);
    if (valueType == Type::getInt32Ty(*context)) {
        
        Constant* formatStr = ConstantDataArray::getString(*context, "%d\n", true);
        GlobalVariable* formatGV = new GlobalVariable(
            *module,
            formatStr->getType(),
            true,
            GlobalValue::PrivateLinkage,
            formatStr,
            ".int_fmt"
        );
        Value* formatPtr = builder->CreateGEP(
            formatStr->getType(),
            formatGV,
            {ConstantInt::get(int32Ty, 0), ConstantInt::get(int32Ty, 0)},
            "int_fmt"
        );
        builder->CreateCall(module->getFunction("printf"), {formatPtr, value});
    }
    else if (valueType == Type::getInt1Ty(*context)) {
        Constant* formatStr = ConstantDataArray::getString(*context, "%d\n", true);
        GlobalVariable* formatGV = new GlobalVariable(
            *module,
            formatStr->getType(),
            true,
            GlobalValue::PrivateLinkage,
            formatStr,
            ".bool_fmt"
        );
        Value* formatPtr = builder->CreateGEP(
            formatStr->getType(),
            formatGV,
            {ConstantInt::get(int32Ty, 0), ConstantInt::get(int32Ty, 0)},
            "bool_fmt"
        );
        Value* extValue = builder->CreateZExt(value, int32Ty, "bool_ext");
        builder->CreateCall(module->getFunction("printf"), {formatPtr, extValue});
    }
    else if (valueType == PointerType::get(Type::getInt8Ty(*context), 0)) {
        Constant* formatStr = ConstantDataArray::getString(*context, "%s\n", true);
        GlobalVariable* formatGV = new GlobalVariable(
            *module,
            formatStr->getType(),
            true,
            GlobalValue::PrivateLinkage,
            formatStr,
            ".str_fmt"
        );
        Value* formatPtr = builder->CreateGEP(
            formatStr->getType(),
            formatGV,
            {ConstantInt::get(int32Ty, 0), ConstantInt::get(int32Ty, 0)},
            "str_fmt"
        );
        builder->CreateCall(module->getFunction("printf"), {formatPtr, value});
    }
}

void CodeGen::generateBlock(BlockNode* blockNode) {
    // Iterate through all the statements in the block and generate them
    for (auto& statement : blockNode->statements) {
        generateStatement(statement.get());
    }
}

void CodeGen::generateLoop(LoopNode* node) {
    Function* func = builder->GetInsertBlock()->getParent();
    BasicBlock* loopStart = BasicBlock::Create(*context, "loop_start", func);
    BasicBlock* loopBody = BasicBlock::Create(*context, "loop_body", func);
    BasicBlock* loopEnd = BasicBlock::Create(*context, "loop_end", func);

    if (node->type == LoopType::For) {
        if (node->init) generateStatement(node->init.get());
        builder->CreateBr(loopStart);

        builder->SetInsertPoint(loopStart);
        Value* cond = node->condition ? generateValue(node->condition.get(), Type::getInt1Ty(*context))
                                      : ConstantInt::getTrue(*context);
        builder->CreateCondBr(cond, loopBody, loopEnd);

        builder->SetInsertPoint(loopBody);
        generateStatement(node->body.get());
        if (node->update) generateStatement(node->update.get());
        builder->CreateBr(loopStart);

        builder->SetInsertPoint(loopEnd);
    } else { // Foreach
        // Generate collection value
        Value* arrayVal = generateValue(node->collection.get(), PointerType::get(Type::getInt32Ty(*context), 0));
        if (!arrayVal->getType()->isPointerTy()) {
            throw std::runtime_error("Foreach collection must be an array pointer");
        }

        // Determine array size
        uint64_t arraySize = 0;
        if (auto* varRef = dynamic_cast<VarRefNode*>(node->collection.get())) {
            auto sizeIt = arraySizes.find(varRef->name);
            if (sizeIt == arraySizes.end()) {
                throw std::runtime_error("Array size not found for: " + varRef->name);
            }
            arraySize = sizeIt->second;
        } else if (auto* arrLit = dynamic_cast<ArrayLiteralNode*>(node->collection.get())) {
            arraySize = arrLit->elements.size();
        } else if (auto* binOp = dynamic_cast<BinaryOpNode*>(node->collection.get())) {
            if (binOp->op == BinaryOp::MULTIPLY_ARRAY || binOp->op == BinaryOp::ADD_ARRAY ||
                binOp->op == BinaryOp::SUBTRACT_ARRAY || binOp->op == BinaryOp::DIVIDE_ARRAY) {
                if (auto* varRef = dynamic_cast<VarRefNode*>(binOp->left.get())) {
                    auto sizeIt = arraySizes.find(varRef->name);
                    if (sizeIt == arraySizes.end()) {
                        throw std::runtime_error("Array size not found for operation");
                    }
                    arraySize = sizeIt->second;
                } else {
                    throw std::runtime_error("Array operation must use named array");
                }
            } else {
                throw std::runtime_error("Invalid array operation in foreach");
            }
        } else {
            throw std::runtime_error("Foreach collection must be an array variable, literal, or operation");
        }

        AllocaInst* index = builder->CreateAlloca(Type::getInt32Ty(*context), nullptr, "foreach_idx");
        builder->CreateStore(ConstantInt::get(Type::getInt32Ty(*context), 0), index);
        AllocaInst* var = builder->CreateAlloca(Type::getInt32Ty(*context), nullptr, node->varName);
        symbols[node->varName] = var;

        builder->CreateBr(loopStart);
        builder->SetInsertPoint(loopStart);
        Value* idx = builder->CreateLoad(Type::getInt32Ty(*context), index);
        Value* cond = builder->CreateICmpSLT(idx, ConstantInt::get(Type::getInt32Ty(*context), arraySize));
        builder->CreateCondBr(cond, loopBody, loopEnd);

        builder->SetInsertPoint(loopBody);
        Value* elementPtr = builder->CreateGEP(Type::getInt32Ty(*context), arrayVal, idx);
        Value* element = builder->CreateLoad(Type::getInt32Ty(*context), elementPtr);
        builder->CreateStore(element, var);
        if (auto* block = dynamic_cast<BlockNode*>(node->body.get())) {
            generateBlock(block);
        } else {
            throw std::runtime_error("Foreach body must be a BlockNode");
        }
        Value* nextIdx = builder->CreateAdd(idx, ConstantInt::get(Type::getInt32Ty(*context), 1));
        builder->CreateStore(nextIdx, index);
        builder->CreateBr(loopStart);

        builder->SetInsertPoint(loopEnd);
        symbols.erase(node->varName);
    }
}

llvm::Value* CodeGen::generatePow(llvm::Value* base, llvm::Value* exp) {
    if (!base->getType()->isIntegerTy(32) || !exp->getType()->isIntegerTy(32)) {
        throw std::runtime_error("pow arguments must be integers");
    }
    // Initialize result = 1
    llvm::Value* result = llvm::ConstantInt::get(llvm::Type::getInt32Ty(*context), 1);

    // Create basic blocks
    llvm::Function* func = builder->GetInsertBlock()->getParent();
    llvm::BasicBlock* entryBB = builder->GetInsertBlock();
    llvm::BasicBlock* loopBB = llvm::BasicBlock::Create(*context, "pow_loop", func);
    llvm::BasicBlock* bodyBB = llvm::BasicBlock::Create(*context, "pow_body", func);
    llvm::BasicBlock* endBB = llvm::BasicBlock::Create(*context, "pow_end", func);

    // Check if exp >= 0
    llvm::Value* zero = llvm::ConstantInt::get(llvm::Type::getInt32Ty(*context), 0);
    llvm::Value* isNonNeg = builder->CreateICmpSGE(exp, zero, "is_nonneg");
    builder->CreateCondBr(isNonNeg, loopBB, endBB);

    // Loop block
    builder->SetInsertPoint(loopBB);
    llvm::PHINode* phi_result = builder->CreatePHI(llvm::Type::getInt32Ty(*context), 2, "result");
    llvm::PHINode* phi_exp = builder->CreatePHI(llvm::Type::getInt32Ty(*context), 2, "exp");
    phi_result->addIncoming(result, entryBB); // From entry
    phi_exp->addIncoming(exp, entryBB);      // From entry

    // Loop condition: exp > 0
    llvm::Value* cond = builder->CreateICmpSGT(phi_exp, zero, "exp_positive");
    builder->CreateCondBr(cond, bodyBB, endBB);

    // Body: result *= base, exp--
    builder->SetInsertPoint(bodyBB);
    llvm::Value* new_result = builder->CreateMul(phi_result, base, "pow_mult");
    llvm::Value* new_exp = builder->CreateSub(phi_exp, llvm::ConstantInt::get(llvm::Type::getInt32Ty(*context), 1), "exp_decr");
    phi_result->addIncoming(new_result, bodyBB); // Loop back
    phi_exp->addIncoming(new_exp, bodyBB);      // Loop back
    builder->CreateBr(loopBB);

    // End block
    builder->SetInsertPoint(endBB);
    llvm::PHINode* final_result = builder->CreatePHI(llvm::Type::getInt32Ty(*context), 2, "final_result");
    final_result->addIncoming(result, entryBB);   // exp < 0
    final_result->addIncoming(phi_result, loopBB); // exp <= 0
    return final_result;
}

void CodeGen::printArray(const std::vector<llvm::Value*>& elements) {
    Type* int32Ty = Type::getInt32Ty(*context);
    Constant* openBracket = ConstantDataArray::getString(*context, "[", true);
    GlobalVariable* openGV = new GlobalVariable(
        *module, openBracket->getType(), true, GlobalValue::PrivateLinkage, openBracket, ".arr_open");
    Value* openPtr = builder->CreateGEP(
        openBracket->getType(), openGV, {ConstantInt::get(int32Ty, 0), ConstantInt::get(int32Ty, 0)});
    builder->CreateCall(module->getFunction("printf"), {openPtr});
    for (size_t i = 0; i < elements.size(); ++i) {
        Constant* formatStr = ConstantDataArray::getString(*context, i < elements.size() - 1 ? "%d, " : "%d", true);
        GlobalVariable* formatGV = new GlobalVariable(
            *module, formatStr->getType(), true, GlobalValue::PrivateLinkage, formatStr, ".arr_elem");
        Value* formatPtr = builder->CreateGEP(
            formatStr->getType(), formatGV, {ConstantInt::get(int32Ty, 0), ConstantInt::get(int32Ty, 0)});
        builder->CreateCall(module->getFunction("printf"), {formatPtr, elements[i]});
    }
    Constant* closeBracket = ConstantDataArray::getString(*context, "]\n", true);
    GlobalVariable* closeGV = new GlobalVariable(
        *module, closeBracket->getType(), true, GlobalValue::PrivateLinkage, closeBracket, ".arr_close");
    Value* closePtr = builder->CreateGEP(
        closeBracket->getType(), closeGV, {ConstantInt::get(int32Ty, 0), ConstantInt::get(int32Ty, 0)});
    builder->CreateCall(module->getFunction("printf"), {closePtr});
}

void CodeGen::printArrayVar(llvm::Value* arrayPtr, uint64_t size) {
    Type* int32Ty = Type::getInt32Ty(*context);
    Constant* openBracket = ConstantDataArray::getString(*context, "[", true);
    GlobalVariable* openGV = new GlobalVariable(
        *module, openBracket->getType(), true, GlobalValue::PrivateLinkage, openBracket, ".arr_open");
    Value* openPtr = builder->CreateGEP(
        openBracket->getType(), openGV, {ConstantInt::get(int32Ty, 0), ConstantInt::get(int32Ty, 0)});
    builder->CreateCall(module->getFunction("printf"), {openPtr});
    Function* func = builder->GetInsertBlock()->getParent();
    BasicBlock* loopStart = BasicBlock::Create(*context, "arr_print_loop", func);
    BasicBlock* loopBody = BasicBlock::Create(*context, "arr_print_body", func);
    BasicBlock* loopEnd = BasicBlock::Create(*context, "arr_print_end", func);
    AllocaInst* index = builder->CreateAlloca(int32Ty, nullptr, "print_idx");
    builder->CreateStore(ConstantInt::get(int32Ty, 0), index);
    builder->CreateBr(loopStart);
    builder->SetInsertPoint(loopStart);
    Value* idx = builder->CreateLoad(int32Ty, index);
    Value* cond = builder->CreateICmpSLT(idx, ConstantInt::get(int32Ty, size));
    builder->CreateCondBr(cond, loopBody, loopEnd);
    builder->SetInsertPoint(loopBody);
    Value* elemPtr = builder->CreateGEP(Type::getInt32Ty(*context), arrayPtr, idx);
    Value* elem = builder->CreateLoad(Type::getInt32Ty(*context), elemPtr);
    Constant* formatStr = ConstantDataArray::getString(*context, "%d", true);
    GlobalVariable* formatGV = new GlobalVariable(
        *module, formatStr->getType(), true, GlobalValue::PrivateLinkage, formatStr, ".arr_elem");
    Value* formatPtr = builder->CreateGEP(
        formatStr->getType(), formatGV, {ConstantInt::get(int32Ty, 0), ConstantInt::get(int32Ty, 0)});
    builder->CreateCall(module->getFunction("printf"), {formatPtr, elem});
    Value* isNotLast = builder->CreateICmpSLT(
        idx, ConstantInt::get(int32Ty, size - 1));
    BasicBlock* commaBB = BasicBlock::Create(*context, "print_comma", func);
    BasicBlock* afterCommaBB = BasicBlock::Create(*context, "after_comma", func);
    builder->CreateCondBr(isNotLast, commaBB, afterCommaBB);
    builder->SetInsertPoint(commaBB);
    Constant* commaStr = ConstantDataArray::getString(*context, ", ", true);
    GlobalVariable* commaGV = new GlobalVariable(
        *module, commaStr->getType(), true, GlobalValue::PrivateLinkage, commaStr, ".comma");
    Value* commaPtr = builder->CreateGEP(
        commaStr->getType(), commaGV, {ConstantInt::get(int32Ty, 0), ConstantInt::get(int32Ty, 0)});
    builder->CreateCall(module->getFunction("printf"), {commaPtr});
    builder->CreateBr(afterCommaBB);
    builder->SetInsertPoint(afterCommaBB);
    Value* nextIdx = builder->CreateAdd(idx, ConstantInt::get(int32Ty, 1));
    builder->CreateStore(nextIdx, index);
    builder->CreateBr(loopStart);
    builder->SetInsertPoint(loopEnd);
    Constant* closeBracket = ConstantDataArray::getString(*context, "]\n", true);
    GlobalVariable* closeGV = new GlobalVariable(
        *module, closeBracket->getType(), true, GlobalValue::PrivateLinkage, closeBracket, ".arr_close");
    Value* closePtr = builder->CreateGEP(
        closeBracket->getType(), closeGV, {ConstantInt::get(int32Ty, 0), ConstantInt::get(int32Ty, 0)});
    builder->CreateCall(module->getFunction("printf"), {closePtr});
}

llvm::Value* CodeGen::generateValue(ASTNode* node, llvm::Type* expectedType) {
    if (auto intLit = dynamic_cast<IntLiteral*>(node)) {
        if (expectedType->isIntegerTy()) {
            return ConstantInt::get(Type::getInt32Ty(*context), intLit->value);
        }
        else if (expectedType->isFloatTy()) {
            return ConstantFP::get(Type::getFloatTy(*context), static_cast<double>(intLit->value));
        }
        throw std::runtime_error("Type mismatch: cannot convert integer to target type");
    }
    else if (auto floatLit = dynamic_cast<FloatLiteral*>(node)) {
        if (!expectedType->isFloatTy()) {
            throw std::runtime_error("Expected float type");
        }
        return ConstantFP::get(Type::getFloatTy(*context), floatLit->value);
    }
    else if (auto strLit = dynamic_cast<StrLiteral*>(node)) {
        if (!expectedType->isPointerTy()) {
            throw std::runtime_error("Expected pointer type for string");
        }
        return builder->CreateGlobalStringPtr(strLit->value);
    }
    else if (auto boolLit = dynamic_cast<BoolLiteral*>(node)) {
        if (!expectedType->isIntegerTy(1)) {
            throw std::runtime_error("Expected boolean type");
        }
        return ConstantInt::get(Type::getInt1Ty(*context), boolLit->value);
    }
    else if (auto charLit = dynamic_cast<CharLiteral*>(node)) {
        if (!expectedType->isIntegerTy(8)) {
            throw std::runtime_error("Expected char (i8) type");
        }
        return ConstantInt::get(Type::getInt8Ty(*context), charLit->value);
    }
    else if (auto arrLit = dynamic_cast<ArrayLiteralNode*>(node)) {
        if (!expectedType->isPointerTy()) {
            throw std::runtime_error("Expected pointer type for array");
        }
        size_t size = arrLit->elements.size();
        Type* elemType = Type::getInt32Ty(*context);
        Value* sizeVal = ConstantInt::get(Type::getInt32Ty(*context), size * 4);
        Value* mallocCall = builder->CreateCall(module->getFunction("malloc"), sizeVal);
        Value* arrayPtr = builder->CreateBitCast(mallocCall, PointerType::get(elemType, 0));
        for (size_t i = 0; i < size; ++i) {
            Value* idx = ConstantInt::get(Type::getInt32Ty(*context), i);
            Value* elemPtr = builder->CreateGEP(elemType, arrayPtr, idx);
            Value* elemVal = generateValue(arrLit->elements[i].get(), elemType);
            builder->CreateStore(elemVal, elemPtr);
        }
        return arrayPtr;
    }
    else if (auto binOp = dynamic_cast<BinaryOpNode*>(node)) {
        // Handle comparison and logical operators (expect i1 output)
        if (binOp->op == BinaryOp::EQUAL || binOp->op == BinaryOp::LESS_EQUAL ||
            binOp->op == BinaryOp::NOT_EQUAL || binOp->op == BinaryOp::GREATER ||
            binOp->op == BinaryOp::GREATER_EQUAL || binOp->op == BinaryOp::LESS ||
            binOp->op == BinaryOp::AND || binOp->op == BinaryOp::OR || binOp->op == BinaryOp::POW ){
            Value* left = generateValue(binOp->left.get(), binOp->op == BinaryOp::AND || binOp->op == BinaryOp::OR ? Type::getInt1Ty(*context) : Type::getInt32Ty(*context));
            Value* right = generateValue(binOp->right.get(), binOp->op == BinaryOp::AND || binOp->op == BinaryOp::OR ? Type::getInt1Ty(*context) : Type::getInt32Ty(*context));
            switch (binOp->op) {
                case BinaryOp::EQUAL: return builder->CreateICmpEQ(left, right);
                case BinaryOp::LESS_EQUAL: return builder->CreateICmpSLE(left, right);
                case BinaryOp::NOT_EQUAL: return builder->CreateICmpNE(left, right);
                case BinaryOp::GREATER: return builder->CreateICmpSGT(left, right);
                case BinaryOp::GREATER_EQUAL: return builder->CreateICmpSGE(left, right);
                case BinaryOp::LESS: return builder->CreateICmpSLT(left, right);
                case BinaryOp::AND: return builder->CreateAnd(left, right);
                case BinaryOp::OR: return builder->CreateOr(left, right);
                case BinaryOp::POW: return generatePow(left, right);
                default: throw std::runtime_error("Unreachable");
            }
        }
        else if (binOp->op == BinaryOp::INDEX) {
            Value* arrayPtr = generateValue(binOp->left.get(), PointerType::get(Type::getInt32Ty(*context), 0));
            Value* index = generateValue(binOp->right.get(), Type::getInt32Ty(*context));
            Value* elemPtr = builder->CreateGEP(Type::getInt32Ty(*context), arrayPtr, index);
            return builder->CreateLoad(Type::getInt32Ty(*context), elemPtr);
        } else if (binOp->op == BinaryOp::MULTIPLY_ARRAY || binOp->op == BinaryOp::ADD_ARRAY ||
                   binOp->op == BinaryOp::SUBTRACT_ARRAY || binOp->op == BinaryOp::DIVIDE_ARRAY) {
            Value* arr1 = generateValue(binOp->left.get(), PointerType::get(Type::getInt32Ty(*context), 0));
            Value* arr2 = generateValue(binOp->right.get(), PointerType::get(Type::getInt32Ty(*context), 0));
            uint64_t size = 5;
            if (auto* varRef = dynamic_cast<VarRefNode*>(binOp->left.get())) {
                auto sizeIt = arraySizes.find(varRef->name);
                if (sizeIt != arraySizes.end()) size = sizeIt->second;
            }
            Type* elemType = Type::getInt32Ty(*context);
            Value* sizeVal = ConstantInt::get(Type::getInt32Ty(*context), size * 4);
            Value* resultPtr = builder->CreateCall(module->getFunction("malloc"), sizeVal);
            resultPtr = builder->CreateBitCast(resultPtr, PointerType::get(elemType, 0));
            Function* func = builder->GetInsertBlock()->getParent();
            BasicBlock* loopStart = BasicBlock::Create(*context, "arr_op_loop", func);
            BasicBlock* loopBody = BasicBlock::Create(*context, "arr_op_body", func);
            BasicBlock* loopEnd = BasicBlock::Create(*context, "arr_op_end", func);
            AllocaInst* index = builder->CreateAlloca(Type::getInt32Ty(*context), nullptr, "op_idx");
            builder->CreateStore(ConstantInt::get(Type::getInt32Ty(*context), 0), index);
            builder->CreateBr(loopStart);
            builder->SetInsertPoint(loopStart);
            Value* idx = builder->CreateLoad(Type::getInt32Ty(*context), index);
            Value* cond = builder->CreateICmpSLT(idx, ConstantInt::get(Type::getInt32Ty(*context), size));
            builder->CreateCondBr(cond, loopBody, loopEnd);
            builder->SetInsertPoint(loopBody);
            Value* elem1Ptr = builder->CreateGEP(elemType, arr1, idx);
            Value* elem2Ptr = builder->CreateGEP(elemType, arr2, idx);
            Value* elem1 = builder->CreateLoad(elemType, elem1Ptr);
            Value* elem2 = builder->CreateLoad(elemType, elem2Ptr);
            Value* resultElem = nullptr;
            switch (binOp->op) {
                case BinaryOp::MULTIPLY_ARRAY: resultElem = builder->CreateMul(elem1, elem2); break;
                case BinaryOp::ADD_ARRAY: resultElem = builder->CreateAdd(elem1, elem2); break;
                case BinaryOp::SUBTRACT_ARRAY: resultElem = builder->CreateSub(elem1, elem2); break;
                case BinaryOp::DIVIDE_ARRAY: resultElem = builder->CreateSDiv(elem1, elem2); break;
                default: throw std::runtime_error("Unreachable");
            }
            Value* resultElemPtr = builder->CreateGEP(elemType, resultPtr, idx);
            builder->CreateStore(resultElem, resultElemPtr);
            Value* nextIdx = builder->CreateAdd(idx, ConstantInt::get(Type::getInt32Ty(*context), 1));
            builder->CreateStore(nextIdx, index);
            builder->CreateBr(loopStart);
            builder->SetInsertPoint(loopEnd);
            return resultPtr;
        }
        // Arithmetic operators
        Value* left = generateValue(binOp->left.get(), expectedType);
        if (binOp->op == BinaryOp::ABS) {
            if (!left->getType()->isIntegerTy(32)) {
                throw std::runtime_error("abs argument must be an integer");
            }
            // abs: x < 0 ? -x : x
            llvm::Value* zero = llvm::ConstantInt::get(llvm::Type::getInt32Ty(*context), 0);
            llvm::Value* isNeg = builder->CreateICmpSLT(left, zero, "is_neg");
            llvm::Value* negExpr = builder->CreateSub(zero, left, "neg");
            return builder->CreateSelect(isNeg, negExpr, left, "abs");
        }
        Value* right = generateValue(binOp->right.get(), expectedType);
        switch (binOp->op) {
            case BinaryOp::ADD:
                return expectedType->isFloatTy() ? builder->CreateFAdd(left, right)
                                                : builder->CreateAdd(left, right);
            case BinaryOp::SUBTRACT:
                return expectedType->isFloatTy() ? builder->CreateFSub(left, right)
                                                : builder->CreateSub(left, right);
            case BinaryOp::MULTIPLY:
                return expectedType->isFloatTy() ? builder->CreateFMul(left, right)
                                                : builder->CreateMul(left, right);
            case BinaryOp::DIVIDE:
                return expectedType->isFloatTy() ? builder->CreateFDiv(left, right)
                                                : builder->CreateSDiv(left, right);
            default:
                throw std::runtime_error("Unsupported binary operator");
        }
    }
    else if (auto varRef = dynamic_cast<VarRefNode*>(node)) {
        auto it = symbols.find(varRef->name);
        if (it == symbols.end()) {
            throw std::runtime_error("Undeclared variable: " + varRef->name);
        }
        AllocaInst* alloca = it->second;
        if (expectedType == PointerType::get(Type::getInt32Ty(*context), 0)) {
            return builder->CreateLoad(expectedType, alloca);
        }
        if (alloca->getAllocatedType() != expectedType) {
            throw std::runtime_error("Type mismatch: variable " + varRef->name + " has a different type");
        }
        return builder->CreateLoad(expectedType, alloca);
    }
    else if (auto unaryOp = dynamic_cast<UnaryOpNode*>(node)) {
        Value* operand = generateValue(unaryOp->operand.get(), PointerType::get(Type::getInt32Ty(*context), 0));
        uint64_t size = 5;
        if (auto* varRef = dynamic_cast<VarRefNode*>(unaryOp->operand.get())) {
            auto sizeIt = arraySizes.find(varRef->name);
            if (sizeIt != arraySizes.end()) size = sizeIt->second;
        }
        switch (unaryOp->op) {
            case UnaryOp::LENGTH:
                return ConstantInt::get(Type::getInt32Ty(*context), size);
            case UnaryOp::MIN: {
                Function* func = builder->GetInsertBlock()->getParent();
                BasicBlock* loopStart = BasicBlock::Create(*context, "min_loop", func);
                BasicBlock* loopBody = BasicBlock::Create(*context, "min_body", func);
                BasicBlock* loopEnd = BasicBlock::Create(*context, "min_end", func);
                AllocaInst* index = builder->CreateAlloca(Type::getInt32Ty(*context), nullptr, "min_idx");
                AllocaInst* minVal = builder->CreateAlloca(Type::getInt32Ty(*context), nullptr, "min_val");
                builder->CreateStore(ConstantInt::get(Type::getInt32Ty(*context), 0), index);
                Value* firstElemPtr = builder->CreateGEP(Type::getInt32Ty(*context), operand, ConstantInt::get(Type::getInt32Ty(*context), 0));
                Value* firstElem = builder->CreateLoad(Type::getInt32Ty(*context), firstElemPtr);
                builder->CreateStore(firstElem, minVal);
                builder->CreateBr(loopStart);
                builder->SetInsertPoint(loopStart);
                Value* idx = builder->CreateLoad(Type::getInt32Ty(*context), index);
                Value* cond = builder->CreateICmpSLT(idx, ConstantInt::get(Type::getInt32Ty(*context), size));
                builder->CreateCondBr(cond, loopBody, loopEnd);
                builder->SetInsertPoint(loopBody);
                Value* elemPtr = builder->CreateGEP(Type::getInt32Ty(*context), operand, idx);
                Value* elem = builder->CreateLoad(Type::getInt32Ty(*context), elemPtr);
                Value* currentMin = builder->CreateLoad(Type::getInt32Ty(*context), minVal);
                Value* isLess = builder->CreateICmpSLT(elem, currentMin);
                Value* newMin = builder->CreateSelect(isLess, elem, currentMin);
                builder->CreateStore(newMin, minVal);
                Value* nextIdx = builder->CreateAdd(idx, ConstantInt::get(Type::getInt32Ty(*context), 1));
                builder->CreateStore(nextIdx, index);
                builder->CreateBr(loopStart);
                builder->SetInsertPoint(loopEnd);
                return builder->CreateLoad(Type::getInt32Ty(*context), minVal);
            }
            case UnaryOp::MAX: {
                Function* func = builder->GetInsertBlock()->getParent();
                BasicBlock* loopStart = BasicBlock::Create(*context, "max_loop", func);
                BasicBlock* loopBody = BasicBlock::Create(*context, "max_body", func);
                BasicBlock* loopEnd = BasicBlock::Create(*context, "max_end", func);
                AllocaInst* index = builder->CreateAlloca(Type::getInt32Ty(*context), nullptr, "max_idx");
                AllocaInst* maxVal = builder->CreateAlloca(Type::getInt32Ty(*context), nullptr, "max_val");
                builder->CreateStore(ConstantInt::get(Type::getInt32Ty(*context), 0), index);
                Value* firstElemPtr = builder->CreateGEP(Type::getInt32Ty(*context), operand, ConstantInt::get(Type::getInt32Ty(*context), 0));
                Value* firstElem = builder->CreateLoad(Type::getInt32Ty(*context), firstElemPtr);
                builder->CreateStore(firstElem, maxVal);
                builder->CreateBr(loopStart);
                builder->SetInsertPoint(loopStart);
                Value* idx = builder->CreateLoad(Type::getInt32Ty(*context), index);
                Value* cond = builder->CreateICmpSLT(idx, ConstantInt::get(Type::getInt32Ty(*context), size));
                builder->CreateCondBr(cond, loopBody, loopEnd);
                builder->SetInsertPoint(loopBody);
                Value* elemPtr = builder->CreateGEP(Type::getInt32Ty(*context), operand, idx);
                Value* elem = builder->CreateLoad(Type::getInt32Ty(*context), elemPtr);
                Value* currentMax = builder->CreateLoad(Type::getInt32Ty(*context), maxVal);
                Value* isGreater = builder->CreateICmpSGT(elem, currentMax);
                Value* newMax = builder->CreateSelect(isGreater, elem, currentMax);
                builder->CreateStore(newMax, maxVal);
                Value* nextIdx = builder->CreateAdd(idx, ConstantInt::get(Type::getInt32Ty(*context), 1));
                builder->CreateStore(nextIdx, index);
                builder->CreateBr(loopStart);
                builder->SetInsertPoint(loopEnd);
                return builder->CreateLoad(Type::getInt32Ty(*context), maxVal);
            }
            default: throw std::runtime_error("Unsupported unary operator");
        }
    }
    else if (auto concat = dynamic_cast<ConcatNode*>(node)) {
        llvm::Type* int8PtrTy = llvm::PointerType::get(llvm::Type::getInt8Ty(*context), 0);
        llvm::Type* int32Ty = llvm::Type::getInt32Ty(*context);

        // Generate values for left and right expressions
        llvm::Value* leftStr = generateValue(concat->left.get(), int8PtrTy);
        llvm::Value* rightStr = generateValue(concat->right.get(), int8PtrTy);

        // Compute lengths
        llvm::Function* strlenFunc = module->getFunction("strlen");
        if (!strlenFunc) throw std::runtime_error("strlen function not found");
        llvm::Value* leftLen = builder->CreateCall(strlenFunc, leftStr, "leftLen");
        llvm::Value* rightLen = builder->CreateCall(strlenFunc, rightStr, "rightLen");

        // Total length including null terminator
        llvm::Value* totalLenNoNull = builder->CreateAdd(leftLen, rightLen, "totalLenNoNull");
        llvm::Value* totalLen = builder->CreateAdd(
            totalLenNoNull,
            llvm::ConstantInt::get(int32Ty, 1),
            "totalLen"
        );

        // Allocate memory
        llvm::Function* mallocFunc = module->getFunction("malloc");
        if (!mallocFunc) throw std::runtime_error("malloc function not found");
        llvm::Value* resultPtr = builder->CreateCall(mallocFunc, totalLen, "concatResult");

        // Copy left string
        llvm::Function* memcpyFunc = module->getFunction("memcpy");
        if (!memcpyFunc) throw std::runtime_error("memcpy function not found");
        builder->CreateCall(memcpyFunc, {
            resultPtr,
            leftStr,
            builder->CreateAdd(leftLen, llvm::ConstantInt::get(int32Ty, 1))
        });

        // Copy right string
        llvm::Value* destPtr = builder->CreateGEP(
            llvm::Type::getInt8Ty(*context),
            resultPtr,
            leftLen,
            "destOffset"
        );
        builder->CreateCall(memcpyFunc, {
            destPtr,
            rightStr,
            builder->CreateAdd(rightLen, llvm::ConstantInt::get(int32Ty, 1))
        });

        return resultPtr;
    }
    throw std::runtime_error("Unsupported value type in code generation");
}

void CodeGen::dump() const {
    module->print(llvm::outs(), nullptr);
}