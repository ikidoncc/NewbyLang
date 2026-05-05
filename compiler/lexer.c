#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "lexer.h"

Lexer *lexer_new(const char *src) {
    Lexer *lexer = malloc(sizeof(Lexer));
    lexer->src = src;
    lexer->pos = 0;
    lexer->line = 1;
    lexer->col = 1;
    return lexer;
}

static Token create_token(TokenType type, const char *value, int line, int col) {
    Token token;
    token.type = type;
    token.value = value ? strdup(value) : NULL;
    token.line = line;
    token.col = col;
    return token;
}

void token_free(Token token) {
    if (token.value) free(token.value);
}

Token lexer_next_token(Lexer *lexer) {
    while (lexer->src[lexer->pos] != '\0' && isspace(lexer->src[lexer->pos])) {
        if (lexer->src[lexer->pos] == '\n') {
            lexer->line++;
            lexer->col = 1;
        } else {
            lexer->col++;
        }
        lexer->pos++;
    }

    int start_line = lexer->line;
    int start_col = lexer->col;
    char c = lexer->src[lexer->pos];

    if (c == '\0') return create_token(TOKEN_EOF, NULL, start_line, start_col);

    if (isdigit(c)) {
        int start = lexer->pos;
        int is_float = 0;
        while (isdigit(lexer->src[lexer->pos]) || lexer->src[lexer->pos] == '.') {
            if (lexer->src[lexer->pos] == '.') is_float = 1;
            lexer->pos++;
            lexer->col++;
        }
        int len = lexer->pos - start;
        char *val = malloc(len + 1);
        strncpy(val, lexer->src + start, len);
        val[len] = '\0';
        Token t = create_token(is_float ? TOKEN_FLOAT_LIT : TOKEN_NUMBER, val, start_line, start_col);
        free(val);
        return t;
    }

    if (isalpha(c)) {
        int start = lexer->pos;
        while (isalnum(lexer->src[lexer->pos])) {
            lexer->pos++;
            lexer->col++;
        }
        int len = lexer->pos - start;
        char *val = malloc(len + 1);
        strncpy(val, lexer->src + start, len);
        val[len] = '\0';

        Token t;
        if (strcmp(val, "int") == 0) t = create_token(TOKEN_INT, NULL, start_line, start_col);
        else if (strcmp(val, "bool") == 0) t = create_token(TOKEN_BOOL, NULL, start_line, start_col);
        else if (strcmp(val, "float") == 0) t = create_token(TOKEN_FLOAT, NULL, start_line, start_col);
        else if (strcmp(val, "string") == 0) t = create_token(TOKEN_STRING, NULL, start_line, start_col);
        else if (strcmp(val, "int8") == 0) t = create_token(TOKEN_INT8, NULL, start_line, start_col);
        else if (strcmp(val, "int32") == 0) t = create_token(TOKEN_INT32, NULL, start_line, start_col);
        else if (strcmp(val, "uint") == 0) t = create_token(TOKEN_UINT, NULL, start_line, start_col);
        else if (strcmp(val, "uint8") == 0) t = create_token(TOKEN_UINT8, NULL, start_line, start_col);
        else if (strcmp(val, "uint32") == 0) t = create_token(TOKEN_UINT32, NULL, start_line, start_col);
        else if (strcmp(val, "true") == 0) t = create_token(TOKEN_TRUE, NULL, start_line, start_col);
        else if (strcmp(val, "false") == 0) t = create_token(TOKEN_FALSE, NULL, start_line, start_col);
        else if (strcmp(val, "if") == 0) t = create_token(TOKEN_IF, NULL, start_line, start_col);
        else if (strcmp(val, "else") == 0) t = create_token(TOKEN_ELSE, NULL, start_line, start_col);
        else if (strcmp(val, "while") == 0) t = create_token(TOKEN_WHILE, NULL, start_line, start_col);
        else if (strcmp(val, "func") == 0) t = create_token(TOKEN_FUNC, NULL, start_line, start_col);
        else if (strcmp(val, "extern") == 0) t = create_token(TOKEN_EXTERN, NULL, start_line, start_col);
        else if (strcmp(val, "return") == 0) t = create_token(TOKEN_RETURN, NULL, start_line, start_col);
        else if (strcmp(val, "match") == 0) t = create_token(TOKEN_MATCH, NULL, start_line, start_col);
        else if (strcmp(val, "case") == 0) t = create_token(TOKEN_CASE, NULL, start_line, start_col);
        else if (strcmp(val, "default") == 0) t = create_token(TOKEN_DEFAULT, NULL, start_line, start_col);
        else if (strcmp(val, "print") == 0) t = create_token(TOKEN_PRINT, NULL, start_line, start_col);
        else t = create_token(TOKEN_ID, val, start_line, start_col);

        free(val);
        return t;
    }

    if (c == '"') {
        lexer->pos++;
        lexer->col++;
        int start = lexer->pos;
        while (lexer->src[lexer->pos] != '"' && lexer->src[lexer->pos] != '\0') {
            lexer->pos++;
            lexer->col++;
        }
        int len = lexer->pos - start;
        char *val = malloc(len + 1);
        strncpy(val, lexer->src + start, len);
        val[len] = '\0';
        if (lexer->src[lexer->pos] == '"') {
            lexer->pos++;
            lexer->col++;
        }
        Token t = create_token(TOKEN_STRING_LIT, val, start_line, start_col);
        free(val);
        return t;
    }

    char next = lexer->src[lexer->pos + 1];
    lexer->pos++;
    lexer->col++;
    switch (c) {
        case '=': 
            if (next == '=') { lexer->pos++; lexer->col++; return create_token(TOKEN_EQ, NULL, start_line, start_col); }
            return create_token(TOKEN_ASSIGN, NULL, start_line, start_col);
        case '!':
            if (next == '=') { lexer->pos++; lexer->col++; return create_token(TOKEN_NEQ, NULL, start_line, start_col); }
            return create_token(TOKEN_UNKNOWN, NULL, start_line, start_col);
        case '&':
            if (next == '&') { lexer->pos++; lexer->col++; return create_token(TOKEN_AND, NULL, start_line, start_col); }
            return create_token(TOKEN_UNKNOWN, NULL, start_line, start_col);
        case '|':
            if (next == '|') { lexer->pos++; lexer->col++; return create_token(TOKEN_OR, NULL, start_line, start_col); }
            return create_token(TOKEN_UNKNOWN, NULL, start_line, start_col);
        case '<': return create_token(TOKEN_LT, NULL, start_line, start_col);
        case '>': return create_token(TOKEN_GT, NULL, start_line, start_col);
        case '+': return create_token(TOKEN_PLUS, NULL, start_line, start_col);
        case '-': return create_token(TOKEN_MINUS, NULL, start_line, start_col);
        case '*': return create_token(TOKEN_STAR, NULL, start_line, start_col);
        case '/': return create_token(TOKEN_SLASH, NULL, start_line, start_col);
        case ';': return create_token(TOKEN_SEMICOLON, NULL, start_line, start_col);
        case ':': return create_token(TOKEN_COLON, NULL, start_line, start_col);
        case ',': return create_token(TOKEN_COMMA, NULL, start_line, start_col);
        case '(': return create_token(TOKEN_LPAREN, NULL, start_line, start_col);
        case ')': return create_token(TOKEN_RPAREN, NULL, start_line, start_col);
        case '[': return create_token(TOKEN_LBRACKET, NULL, start_line, start_col);
        case ']': return create_token(TOKEN_RBRACKET, NULL, start_line, start_col);
        case '{': return create_token(TOKEN_LBRACE, NULL, start_line, start_col);
        case '}': return create_token(TOKEN_RBRACE, NULL, start_line, start_col);
        default: return create_token(TOKEN_UNKNOWN, NULL, start_line, start_col);
    }
}
