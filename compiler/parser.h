#ifndef PARSER_H
#define PARSER_H

#include "lexer.h"
#include "ast.h"

typedef struct {
    Lexer *lexer;
    Token current_token;
    struct ASTNode *root;
} Parser;

Parser *parser_new(Lexer *lexer);
ASTNode *parser_parse(Parser *parser);
void parser_free(Parser *p);

#endif
