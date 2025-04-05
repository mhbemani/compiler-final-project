#ifndef LEXER_H
#define LEXER_H

typedef enum {
    TOK_INT,
    TOK_STRING,
    TOK_ID,
    TOK_EQ,
    TOK_NUM,
    TOK_STR_LIT,
    TOK_SEMI,
    TOK_EOF
} TokenType;

typedef struct {
    TokenType type;
    char* value;
} Token;

Token* tokenize(const char* input);
void free_tokens(Token* tokens);

#endif