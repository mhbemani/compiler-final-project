#ifndef AST_H
#define AST_H

#ifdef __cplusplus
extern "C" {
#endif

typedef enum {
    NODE_INT_LITERAL,
    NODE_STRING_LITERAL,
    NODE_IDENTIFIER,
    NODE_VAR_DECL,
    NODE_VAR_ASSIGN
} NodeType;

typedef struct ASTNode {
    NodeType type;
    char* data_type;
    char* identifier;
    union {
        int int_value;
        char* string_value;
        struct ASTNode* assignment_value;
    };
} ASTNode;

// Function declarations
ASTNode* create_int_node(int value);
ASTNode* create_string_node(char* value);
ASTNode* create_identifier_node(char* id);
ASTNode* create_var_decl_node(char* data_type, char* id, ASTNode* value);
ASTNode* create_var_assign_node(char* id, ASTNode* value);
void free_ast(ASTNode* node);

#ifdef __cplusplus
}
#endif

#endif