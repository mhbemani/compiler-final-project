#ifndef PARSER_H
#define PARSER_H

#include "lexer.h"
#include "ast.h"

typedef struct {
    Lexer* lexer;
    Token current_token;
    char* error;
} Parser;

void init_parser(Parser* parser, Lexer* lexer);
ASTNode* parse_statement(Parser* parser);

#endif