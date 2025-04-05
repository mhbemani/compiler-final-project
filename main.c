#include "lexer.h"
#include "parser.h"
#include "codegen.h"
#include <stdio.h>

int main() {
    const char* input = "int a = 5; b = 3; String s = \"hello\";";
    Token* tokens = tokenize(input);
    
    // Parse each statement (simplified)
    for (int i = 0; tokens[i].type != TOK_EOF; ) {
        ASTNode* ast = parse(&tokens[i]);
        codegen(ast);
        free_ast(ast);
        
        // Advance to next statement (5 tokens per statement in our simple grammar)
        i += 5;
    }
    
    free_tokens(tokens);
    return 0;
}