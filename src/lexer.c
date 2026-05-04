#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "lexer.h"

Lexer *lexer_new(const char *src) {
    Lexer *lexer = malloc(sizeof(Lexer));
    lexer->src = src;
    lexer->pos = 0;
    return lexer;
}

static Token create_token(TokenType type, const char *value) {
    Token token;
    token.type = type;
    token.value = value ? strdup(value) : NULL;
    return token;
}

void token_free(Token token) {
    if (token.value) free(token.value);
}

Token lexer_next_token(Lexer *lexer) {
    while (lexer->src[lexer->pos] != '\0' && isspace(lexer->src[lexer->pos])) {
        lexer->pos++;
    }

    char c = lexer->src[lexer->pos];

    if (c == '\0') return create_token(TOKEN_EOF, NULL);

    if (isdigit(c)) {
        int start = lexer->pos;
        while (isdigit(lexer->src[lexer->pos])) lexer->pos++;
        int len = lexer->pos - start;
        char *val = malloc(len + 1);
        strncpy(val, lexer->src + start, len);
        val[len] = '\0';
        Token t = create_token(TOKEN_NUMBER, val);
        free(val);
        return t;
    }

    if (isalpha(c)) {
        int start = lexer->pos;
        while (isalnum(lexer->src[lexer->pos])) lexer->pos++;
        int len = lexer->pos - start;
        char *val = malloc(len + 1);
        strncpy(val, lexer->src + start, len);
        val[len] = '\0';

        Token t;
        if (strcmp(val, "int") == 0) t = create_token(TOKEN_INT, NULL);
        else if (strcmp(val, "print") == 0) t = create_token(TOKEN_PRINT, NULL);
        else t = create_token(TOKEN_ID, val);

        free(val);
        return t;
    }

    lexer->pos++;
    switch (c) {
        case '=': return create_token(TOKEN_ASSIGN, NULL);
        case '+': return create_token(TOKEN_PLUS, NULL);
        case ';': return create_token(TOKEN_SEMICOLON, NULL);
        case '(': return create_token(TOKEN_LPAREN, NULL);
        case ')': return create_token(TOKEN_RPAREN, NULL);
        default: return create_token(TOKEN_UNKNOWN, NULL);
    }
}
