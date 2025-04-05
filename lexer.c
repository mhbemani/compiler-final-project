#include "lexer.h"
#include <ctype.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>

static const char* input = NULL;

Token* tokenize(const char* src) {
    input = src;
    Token* tokens = malloc(32 * sizeof(Token));
    int pos = 0;

    while (*input) {
        if (isspace(*input)) { input++; continue; }

        if (strncmp(input, "int", 3) == 0 && !isalnum(input[3])) {
            tokens[pos++] = (Token){TOK_INT, strdup("int")};
            input += 3;
            continue;
        }

        if (strncmp(input, "String", 6) == 0 && !isalnum(input[6])) {
            tokens[pos++] = (Token){TOK_STRING, strdup("String")};
            input += 6;
            continue;
        }

        if (isalpha(*input)) {
            char* start = input;
            while (isalnum(*input)) input++;
            tokens[pos++] = (Token){TOK_ID, strndup(start, input - start)};
            continue;
        }

        if (isdigit(*input)) {
            char* start = input;
            while (isdigit(*input)) input++;
            tokens[pos++] = (Token){TOK_NUM, strndup(start, input - start)};
            continue;
        }

        if (*input == '"') {
            char* start = ++input;
            while (*input != '"' && *input != '\0') input++;
            if (*input == '\0') {
                fprintf(stderr, "Unterminated string literal\n");
                exit(1);
            }
            tokens[pos++] = (Token){TOK_STR_LIT, strndup(start, input - start)};
            input++;
            continue;
        }

        if (*input == '=') { tokens[pos++] = (Token){TOK_EQ, strdup("=")}; input++; continue; }
        if (*input == ';') { tokens[pos++] = (Token){TOK_SEMI, strdup(";")}; input++; continue; }

        fprintf(stderr, "Unknown token: %c\n", *input);
        exit(1);
    }

    tokens[pos] = (Token){TOK_EOF, NULL};
    return tokens;
}

void free_tokens(Token* tokens) {
    for (Token* t = tokens; t->type != TOK_EOF; t++) {
        if (t->value) free(t->value);
    }
    free(tokens);
}