#include "codegen.h"
#include "ast.h"  // Now included after codegen.h
#include <llvm-c/Core.h>
#include <stdlib.h>

// Use LLVM's official types directly
static LLVMModuleRef module = NULL;
static LLVMBuilderRef builder = NULL;

void init_codegen() {
    module = LLVMModuleCreateWithName("main");
    builder = LLVMCreateBuilder();
}

void codegen_node(ASTNode* node) {
    if (!module || !builder) return;
    
    switch (node->type) {
        case NODE_INT_LITERAL: {
            LLVMValueRef val = LLVMConstInt(LLVMInt32Type(), node->int_value, 0);
            // TODO: Handle other node types
            break;
        }
        case NODE_STRING_LITERAL:
        case NODE_IDENTIFIER:
        case NODE_VAR_DECL:
        case NODE_VAR_ASSIGN:
            // Implement these cases
            break;
    }
}

void finalize_codegen() {
    if (builder) LLVMDisposeBuilder(builder);
    if (module) LLVMDisposeModule(module);
}