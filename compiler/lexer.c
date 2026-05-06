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

static Token create_token(TokenType type, const char *val, int line, int col) {
    Token t;
    t.type = type;
    t.value = val ? strdup(val) : NULL;
    t.line = line;
    t.col = col;
    return t;
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

    if (lexer->src[lexer->pos] == '\0') {
        return create_token(TOKEN_EOF, NULL, start_line, start_col);
    }

    char c = lexer->src[lexer->pos];

    if (isdigit(c)) {
        char val[64];
        int i = 0;
        while (isdigit(lexer->src[lexer->pos]) || lexer->src[lexer->pos] == '.') {
            val[i++] = lexer->src[lexer->pos++];
            lexer->col++;
        }
        val[i] = '\0';
        if (strchr(val, '.')) return create_token(TOKEN_FLOAT_LIT, val, start_line, start_col);
        return create_token(TOKEN_NUMBER, val, start_line, start_col);
    }

    if (isalpha(c) || c == '_') {
        char val[64];
        int i = 0;
        while (isalnum(lexer->src[lexer->pos]) || lexer->src[lexer->pos] == '_') {
            val[i++] = lexer->src[lexer->pos++];
            lexer->col++;
        }
        val[i] = '\0';

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
        else if (strcmp(val, "syscall") == 0) t = create_token(TOKEN_SYSCALL, NULL, start_line, start_col);
        else if (strcmp(val, "import") == 0) t = create_token(TOKEN_IMPORT, NULL, start_line, start_col);
        else if (strcmp(val, "pub") == 0) t = create_token(TOKEN_PUB, NULL, start_line, start_col);
        else if (strcmp(val, "struct") == 0) t = create_token(TOKEN_STRUCT, NULL, start_line, start_col);
        else if (strcmp(val, "sizeof") == 0) t = create_token(TOKEN_SIZEOF, NULL, start_line, start_col);
        else if (strcmp(val, "self") == 0) t = create_token(TOKEN_SELF, NULL, start_line, start_col);
        else if (strcmp(val, "return") == 0) t = create_token(TOKEN_RETURN, NULL, start_line, start_col);
        else if (strcmp(val, "match") == 0) t = create_token(TOKEN_MATCH, NULL, start_line, start_col);
        else if (strcmp(val, "case") == 0) t = create_token(TOKEN_CASE, NULL, start_line, start_col);
        else if (strcmp(val, "default") == 0) t = create_token(TOKEN_DEFAULT, NULL, start_line, start_col);
        else if (strcmp(val, "print") == 0) t = create_token(TOKEN_PRINT, NULL, start_line, start_col);
        else t = create_token(TOKEN_ID, val, start_line, start_col);

        return t;
    }

    if (c == '"') {
        lexer->pos++;
        lexer->col++;
        char val[256];
        int i = 0;
        while (lexer->src[lexer->pos] != '"' && lexer->src[lexer->pos] != '\0') {
            val[i++] = lexer->src[lexer->pos++];
            lexer->col++;
        }
        val[i] = '\0';
        if (lexer->src[lexer->pos] == '"') {
            lexer->pos++;
            lexer->col++;
        }
        return create_token(TOKEN_STRING_LIT, val, start_line, start_col);
    }

    char next = (lexer->src[lexer->pos] != '\0') ? lexer->src[lexer->pos + 1] : '\0';
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
            return create_token(TOKEN_AMPERSAND, NULL, start_line, start_col);
        case '|':
            if (next == '|') { lexer->pos++; lexer->col++; return create_token(TOKEN_OR, NULL, start_line, start_col); }
            return create_token(TOKEN_UNKNOWN, NULL, start_line, start_col);
        case '<': return create_token(TOKEN_LT, NULL, start_line, start_col);
        case '>': return create_token(TOKEN_GT, NULL, start_line, start_col);
        case '+': return create_token(TOKEN_PLUS, NULL, start_line, start_col);
        case '-': return create_token(TOKEN_MINUS, NULL, start_line, start_col);
        case '*': return create_token(TOKEN_STAR, NULL, start_line, start_col);
        case '/': return create_token(TOKEN_SLASH, NULL, start_line, start_col);
        case '.': return create_token(TOKEN_DOT, NULL, start_line, start_col);
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

Token lexer_peek(Lexer *lexer) {
    int old_pos = lexer->pos;
    int old_line = lexer->line;
    int old_col = lexer->col;
    Token t = lexer_next_token(lexer);
    lexer->pos = old_pos;
    lexer->line = old_line;
    lexer->col = old_col;
    return t;
}
