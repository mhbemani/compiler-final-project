#ifndef AST_H
#define AST_H

#include <stdlib.h>
#include <string.h>

typedef enum {
    AST_VARDECL_INT,     // int a = 5;
    AST_VARDECL_STRING,  // String s = "text";
    // Ready for future expansion:
    AST_BINARY_EXPR      // a + b (example for later)
} ASTType;

typedef struct ASTNode {
    ASTType type;
    char* name;
    union {
        int int_val;    // For AST_VARDECL_INT
        char* str_val;  // For AST_VARDECL_STRING
        // Future: struct { ASTNode* left, *right; char* op; } for expressions
    };
} ASTNode;

// Factory functions (defined inline)
static inline ASTNode* create_int_decl(char* name, int value) {
    ASTNode* node = malloc(sizeof(ASTNode));
    node->type = AST_VARDECL_INT;
    node->name = strdup(name);
    node->int_val = value;
    return node;
}

static inline ASTNode* create_str_decl(char* name, char* value) {
    ASTNode* node = malloc(sizeof(ASTNode));
    node->type = AST_VARDECL_STRING;
    node->name = strdup(name);
    node->str_val = strdup(value);
    return node;
}

static inline void free_ast(ASTNode* node) {
    if (!node) return;
    free(node->name);
    if (node->type == AST_VARDECL_STRING) free(node->str_val);
    free(node);
}

#endif // AST_H