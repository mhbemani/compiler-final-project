#include "sema.h"
#include <stdio.h>

void sema_init(SemanticState* state) {
    state->error_count = 0;
}

void sema_check(SemanticState* state, ASTNode* node) {
    // Basic semantic checks can be added here later
    (void)state; (void)node; // Silence unused warnings for now
}

int sema_has_errors(SemanticState* state) {
    return state->error_count > 0;
}