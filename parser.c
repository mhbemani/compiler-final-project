#include "parser.h"
#include <stdio.h>
#include <stdlib.h>

ASTNode* parse(Token* tokens) {
    int pos = 0;
    
    // Handle implicit int (b = 3;)
    if (tokens[pos].type == TOK_ID && 
        tokens[pos+1].type == TOK_EQ && 
        tokens[pos+2].type == TOK_NUM && 
        tokens[pos+3].type == TOK_SEMI) {
        
        return create_int_decl(tokens[pos].value, atoi(tokens[pos+2].value));
    }
    
    // Handle int declaration (int a = 5;)
    if (tokens[pos].type == TOK_INT && 
        tokens[pos+1].type == TOK_ID && 
        tokens[pos+2].type == TOK_EQ && 
        tokens[pos+3].type == TOK_NUM && 
        tokens[pos+4].type == TOK_SEMI) {
        
        return create_int_decl(tokens[pos+1].value, atoi(tokens[pos+3].value));
    }
    
    // Handle String declaration (String s = "text";)
    if (tokens[pos].type == TOK_STRING && 
        tokens[pos+1].type == TOK_ID && 
        tokens[pos+2].type == TOK_EQ && 
        tokens[pos+3].type == TOK_STR_LIT && 
        tokens[pos+4].type == TOK_SEMI) {
        
        return create_str_decl(tokens[pos+1].value, tokens[pos+3].value);
    }

    fprintf(stderr, "Syntax error\n");
    exit(1);
}