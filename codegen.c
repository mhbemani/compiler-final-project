#include "codegen.h"
#include <llvm-c/Core.h>
#include <llvm-c/Analysis.h>
#include <llvm-c/ExecutionEngine.h>
#include <stdio.h>

void codegen(ASTNode* node) {
    // Initialize LLVM (required for C API)
    LLVMInitializeCore(LLVMGetGlobalPassRegistry());
    
    // Create module and builder
    LLVMContextRef context = LLVMGetGlobalContext();
    LLVMModuleRef module = LLVMModuleCreateWithNameInContext("tiny_module", context);
    LLVMBuilderRef builder = LLVMCreateBuilderInContext(context);

    if (node->type == AST_VARDECL_INT) {
        // Create 32-bit integer global
        LLVMTypeRef int_type = LLVMInt32Type();
        LLVMValueRef init_val = LLVMConstInt(int_type, node->int_val, 0);
        LLVMValueRef global_var = LLVMAddGlobal(module, int_type, node->name);
        LLVMSetInitializer(global_var, init_val);
    } 
    else if (node->type == AST_VARDECL_STRING) {
        // Create string global
        LLVMTypeRef char_ptr_type = LLVMPointerType(LLVMInt8Type(), 0);
        LLVMValueRef init_val = LLVMBuildGlobalStringPtr(builder, node->str_val, "str_const");
        LLVMValueRef global_var = LLVMAddGlobal(module, char_ptr_type, node->name);
        LLVMSetInitializer(global_var, init_val);
    }

    // Verify and dump module
    char* verify_error = NULL;
    if (LLVMVerifyModule(module, LLVMAbortProcessAction, &verify_error)) {
        fprintf(stderr, "Verification failed: %s\n", verify_error);
        LLVMDisposeMessage(verify_error);
    }
    LLVMDumpModule(module);

    // Cleanup
    LLVMDisposeBuilder(builder);
    LLVMDisposeModule(module);
    LLVMShutdown();
}