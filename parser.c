#include "parser.h"
#include <stdlib.h>
#include "ast.h"
#include <string.h>

static void advance(Parser* parser) {
    free_token(&parser->current_token);
    parser->current_token = next_token(parser->lexer);
}

static ASTNode* parse_expression(Parser* parser) {
    if (parser->current_token.type == TOKEN_IDENTIFIER) {
        char* identifier = strdup(parser->current_token.lexeme);
        advance(parser);
        return create_identifier_node(identifier);
    }
    if (parser->current_token.type == TOKEN_INT_LITERAL) {
        int value = parser->current_token.int_value;
        advance(parser);
        return create_int_node(value);
    }
    if (parser->current_token.type == TOKEN_STRING_LITERAL) {
        char* value = strdup(parser->current_token.string_value);
        advance(parser);
        return create_string_node(value);
    }
    return NULL;
}

ASTNode* parse_statement(Parser* parser) {
    // Variable declaration
    if (parser->current_token.type == TOKEN_INT || parser->current_token.type == TOKEN_STRING_TYPE) {
        char* data_type = strdup(parser->current_token.type == TOKEN_INT ? "int" : "String");
        advance(parser);
        
        if (parser->current_token.type != TOKEN_IDENTIFIER) {
            parser->error = "Expected identifier";
            return NULL;
        }
        char* identifier = strdup(parser->current_token.lexeme);
        advance(parser);
        
        if (parser->current_token.type != TOKEN_EQUALS) {
            parser->error = "Expected '='";
            return NULL;
        }
        advance(parser);
        
        ASTNode* value = parse_expression(parser);
        if (!value) {
            parser->error = "Expected value";
            return NULL;
        }
        
        if (parser->current_token.type != TOKEN_SEMICOLON) {
            parser->error = "Expected ';'";
            return NULL;
        }
        advance(parser);
        
        return create_var_decl_node(data_type, identifier, value);
    }
    
    // Variable assignment
    if (parser->current_token.type == TOKEN_IDENTIFIER) {
        char* identifier = strdup(parser->current_token.lexeme);
        advance(parser);
        
        if (parser->current_token.type != TOKEN_EQUALS) {
            parser->error = "Expected '='";
            return NULL;
        }
        advance(parser);
        
        ASTNode* value = parse_expression(parser);
        if (!value) {
            parser->error = "Expected value";
            return NULL;
        }
        
        if (parser->current_token.type != TOKEN_SEMICOLON) {
            parser->error = "Expected ';'";
            return NULL;
        }
        advance(parser);
        
        return create_var_assign_node(identifier, value);
    }
    
    parser->error = "Unknown statement";
    return NULL;
}

void init_parser(Parser* parser, Lexer* lexer) {
    parser->lexer = lexer;
    parser->error = NULL;
    advance(parser);
}