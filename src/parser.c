#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "parser.h"

Parser *parser_new(Lexer *lexer) {
    Parser *p = malloc(sizeof(Parser));
    p->lexer = lexer;
    p->current_token = lexer_next_token(lexer);
    return p;
}

static void eat(Parser *p, TokenType type) {
    if (p->current_token.type == type) {
        token_free(p->current_token);
        p->current_token = lexer_next_token(p->lexer);
    } else {
        fprintf(stderr, "Unexpected token: %d, expected: %d\n", p->current_token.type, type);
        exit(1);
    }
}

static ASTNode *parse_expression(Parser *p);

static ASTNode *parse_term(Parser *p) {
    Token t = p->current_token;
    if (t.type == TOKEN_NUMBER) {
        ASTNode *n = ast_new_number(atoi(t.value));
        eat(p, TOKEN_NUMBER);
        return n;
    } else if (t.type == TOKEN_ID) {
        ASTNode *n = ast_new_variable(t.value);
        char *val = t.value;
        eat(p, TOKEN_ID);
        return n;
    } else if (t.type == TOKEN_LPAREN) {
        eat(p, TOKEN_LPAREN);
        ASTNode *n = parse_expression(p);
        eat(p, TOKEN_RPAREN);
        return n;
    }
    return NULL;
}

static ASTNode *parse_expression(Parser *p) {
    ASTNode *left = parse_term(p);
    while (p->current_token.type == TOKEN_PLUS) {
        eat(p, TOKEN_PLUS);
        ASTNode *right = parse_term(p);
        left = ast_new_bin_op("+", left, right);
    }
    return left;
}

static ASTNode *parse_statement(Parser *p) {
    if (p->current_token.type == TOKEN_INT) {
        eat(p, TOKEN_INT);
        char *name = strdup(p->current_token.value);
        eat(p, TOKEN_ID);
        eat(p, TOKEN_ASSIGN);
        ASTNode *val = parse_expression(p);
        eat(p, TOKEN_SEMICOLON);
        return ast_new_var_decl(name, val);
    } else if (p->current_token.type == TOKEN_PRINT) {
        eat(p, TOKEN_PRINT);
        eat(p, TOKEN_LPAREN);
        ASTNode *expr = parse_expression(p);
        eat(p, TOKEN_RPAREN);
        eat(p, TOKEN_SEMICOLON);
        return ast_new_print(expr);
    }
    return NULL;
}

ASTNode *parser_parse(Parser *p) {
    ASTNode *prog = ast_new_program();
    while (p->current_token.type != TOKEN_EOF) {
        ASTNode *stmt = parse_statement(p);
        if (stmt) ast_program_add(prog, stmt);
    }
    return prog;
}
