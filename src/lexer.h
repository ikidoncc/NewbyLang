#ifndef LEXER_H
#define LEXER_H

typedef enum {
    TOKEN_INT,
    TOKEN_ID,
    TOKEN_NUMBER,
    TOKEN_ASSIGN,   // =
    TOKEN_PLUS,     // +
    TOKEN_SEMICOLON, // ;
    TOKEN_LPAREN,   // (
    TOKEN_RPAREN,   // )
    TOKEN_PRINT,
    TOKEN_EOF,
    TOKEN_UNKNOWN
} TokenType;

typedef struct {
    TokenType type;
    char *value;
} Token;

typedef struct {
    const char *src;
    int pos;
} Lexer;

Lexer *lexer_new(const char *src);
Token lexer_next_token(Lexer *lexer);
void token_free(Token token);

#endif
