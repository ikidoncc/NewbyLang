#ifndef LEXER_H
#define LEXER_H

typedef enum {
    TOKEN_INT,
    TOKEN_BOOL,
    TOKEN_FLOAT,
    TOKEN_STRING,
    TOKEN_INT8,
    TOKEN_INT32,
    TOKEN_UINT,
    TOKEN_UINT8,
    TOKEN_UINT32,
    TOKEN_TRUE,
    TOKEN_FALSE,
    TOKEN_IF,
    TOKEN_ELSE,
    TOKEN_WHILE,
    TOKEN_MATCH,
    TOKEN_CASE,
    TOKEN_DEFAULT,
    TOKEN_ID,
    TOKEN_NUMBER,
    TOKEN_FLOAT_LIT,
    TOKEN_STRING_LIT,
    TOKEN_ASSIGN,   // =
    TOKEN_PLUS,     // +
    TOKEN_MINUS,    // -
    TOKEN_STAR,     // *
    TOKEN_SLASH,    // /
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
    int line;
    int col;
} Token;

typedef struct {
    const char *src;
    int pos;
    int line;
    int col;
} Lexer;

Lexer *lexer_new(const char *src);
Token lexer_next_token(Lexer *lexer);
void token_free(Token token);

#endif
