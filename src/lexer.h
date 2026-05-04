#ifndef LEXER_H
#define LEXER_H

typedef enum {
    TOKEN_INT,
    TOKEN_BOOL,
    TOKEN_TRUE,
    TOKEN_FALSE,
    TOKEN_IF,
    TOKEN_ELSE,
    TOKEN_MATCH,
    TOKEN_CASE,
    TOKEN_DEFAULT,
    TOKEN_ID,
    TOKEN_NUMBER,
    TOKEN_ASSIGN,   // =
    TOKEN_PLUS,     // +
    TOKEN_EQ,       // ==
    TOKEN_NEQ,      // !=
    TOKEN_LT,       // <
    TOKEN_GT,       // >
    TOKEN_AND,      // &&
    TOKEN_OR,       // ||
    TOKEN_SEMICOLON, // ;
    TOKEN_COLON,     // :
    TOKEN_LPAREN,   // (
    TOKEN_RPAREN,   // )
    TOKEN_LBRACE,   // {
    TOKEN_RBRACE,   // }
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
