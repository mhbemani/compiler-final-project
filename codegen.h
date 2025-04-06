#ifndef CODEGEN_H
#define CODEGEN_H

#ifdef __cplusplus
extern "C" {
#endif

// Forward declare ASTNode to avoid including ast.h here
typedef struct ASTNode ASTNode;

// Use LLVM's official types (don't redefine)
#include <llvm-c/Types.h>

void init_codegen();
void codegen_node(ASTNode* node);
void finalize_codegen();

#ifdef __cplusplus
}
#endif

#endif