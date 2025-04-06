#include "ast.h"
#include <stdlib.h>
#include <string.h>

ASTNode* create_int_node(int value) {
    ASTNode* node = malloc(sizeof(ASTNode));
    node->type = NODE_INT_LITERAL;
    node->int_value = value;
    return node;
}

ASTNode* create_string_node(char* value) {
    ASTNode* node = malloc(sizeof(ASTNode));
    node->type = NODE_STRING_LITERAL;
    node->string_value = strdup(value);
    return node;
}

ASTNode* create_identifier_node(char* id) {
    ASTNode* node = malloc(sizeof(ASTNode));
    node->type = NODE_IDENTIFIER;
    node->identifier = strdup(id);
    return node;
}

ASTNode* create_var_decl_node(char* data_type, char* id, ASTNode* value) {
    ASTNode* node = malloc(sizeof(ASTNode));
    node->type = NODE_VAR_DECL;
    node->data_type = strdup(data_type);
    node->identifier = strdup(id);
    node->assignment_value = value;
    return node;
}

ASTNode* create_var_assign_node(char* id, ASTNode* value) {
    ASTNode* node = malloc(sizeof(ASTNode));
    node->type = NODE_VAR_ASSIGN;
    node->identifier = strdup(id);
    node->assignment_value = value;
    return node;
}

void free_ast(ASTNode* node) {
    if (!node) return;
    if (node->type == NODE_STRING_LITERAL || node->type == NODE_IDENTIFIER) {
        free(node->string_value);
    }
    if (node->data_type) free(node->data_type);
    if (node->identifier) free(node->identifier);
    free(node);
}