#include <stdio.h>
#include <stdlib.h>
#include "lexer.h"
#include "ast.h"
#include "parser.h"
#include "codegen.h"

int main(int argc, char** argv) {
    if (argc != 2) {
        fprintf(stderr, "Usage: %s <input>\n", argv[0]);
        return 1;
    }

    const char* source = argv[1];
    
    // Initialize code generation
    init_codegen();
    
    // Lexing and parsing
    Lexer lexer;
    init_lexer(&lexer, source);
    
    Parser parser;
    init_parser(&parser, &lexer);
    
    while (parser.current_token.type != TOKEN_EOF) {
        ASTNode* node = parse_statement(&parser);
        if (parser.error) {
            fprintf(stderr, "Parser error: %s\n", parser.error);
            return 1;
        }
        codegen_node(node);
        free_ast(node);
    }
    
    // Finalize code generation
    finalize_codegen();
    
    return 0;
}