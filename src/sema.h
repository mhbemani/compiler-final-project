#ifndef SEMA_H
#define SEMA_H

#include "ast.h"

typedef struct {
    int error_count;
} SemanticState;

void sema_init(SemanticState* state);
void sema_check(SemanticState* state, ASTNode* node);
int sema_has_errors(SemanticState* state);

#endif