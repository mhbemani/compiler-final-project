#ifndef CODEGEN_H
#define CODEGEN_H

#include <llvm-c/Core.h>
#include "ast.h"

void codegen(ASTNode* node);

#endif