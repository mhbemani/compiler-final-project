#include "lexer.h"
#include <ctype.h>
#include <string.h>
#include <stdlib.h>

static void skip_whitespace(Lexer* lexer) {
    while (isspace(*lexer->current)) {
        if (*lexer->current == '\n') lexer->line++;
        lexer->current++;
    }
}

static int is_identifier_char(char c) {
    return isalnum(c) || c == '_';
}

Token next_token(Lexer* lexer) {
    skip_whitespace(lexer);
    
    Token token = {0};
    token.lexeme = (char*)lexer->current;
    
    if (*lexer->current == '\0') {
        token.type = TOKEN_EOF;
        return token;
    }
    
    // Handle identifiers and keywords
    if (isalpha(*lexer->current)) {
        const char* start = lexer->current;
        while (is_identifier_char(*lexer->current)) lexer->current++;
        
        size_t length = lexer->current - start;
        if (length == 3 && strncmp(start, "int", 3) == 0) {
            token.type = TOKEN_INT;
        } else if (length == 6 && strncmp(start, "String", 6) == 0) {
            token.type = TOKEN_STRING_TYPE;
        } else {
            token.type = TOKEN_IDENTIFIER;
            token.lexeme = strndup(start, length);
        }
        return token;
    }
    
    // Handle integers
    if (isdigit(*lexer->current)) {
        token.type = TOKEN_INT_LITERAL;
        token.int_value = strtol(lexer->current, (char**)&lexer->current, 10);
        return token;
    }
    
    // Handle strings
    if (*lexer->current == '"') {
        lexer->current++;
        const char* start = lexer->current;
        while (*lexer->current != '"' && *lexer->current != '\0') lexer->current++;
        if (*lexer->current == '"') {
            token.type = TOKEN_STRING_LITERAL;
            size_t length = lexer->current - start;
            token.string_value = strndup(start, length);
            lexer->current++;
        } else {
            token.type = TOKEN_ERROR;
        }
        return token;
    }
    
    // Handle symbols
    switch (*lexer->current) {
        case '=':
            token.type = TOKEN_EQUALS;
            lexer->current++;
            break;
        case ';':
            token.type = TOKEN_SEMICOLON;
            lexer->current++;
            break;
        default:
            token.type = TOKEN_ERROR;
            lexer->current++;
            break;
    }
    
    return token;
}

void init_lexer(Lexer* lexer, const char* source) {
    lexer->source = source;
    lexer->current = source;
    lexer->line = 1;
}

void free_token(Token* token) {
    if (token->type == TOKEN_IDENTIFIER || token->type == TOKEN_STRING_LITERAL) {
        free(token->lexeme);
    }
}