#ifndef LEXER_H
#define LEXER_H

typedef enum {
    TOKEN_EOF,
    TOKEN_INT,
    TOKEN_STRING_TYPE,
    TOKEN_IDENTIFIER,
    TOKEN_INT_LITERAL,
    TOKEN_STRING_LITERAL,
    TOKEN_EQUALS,
    TOKEN_SEMICOLON,
    TOKEN_ERROR
} TokenType;

typedef struct {
    TokenType type;
    char* lexeme;
    int int_value;
    char* string_value;
} Token;

typedef struct {
    const char* source;
    const char* current;
    int line;
} Lexer;

void init_lexer(Lexer* lexer, const char* source);
Token next_token(Lexer* lexer);
void free_token(Token* token);

#endif