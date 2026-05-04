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

static ASTNode *parse_primary(Parser *p) {
    Token t = p->current_token;
    if (t.type == TOKEN_NUMBER) {
        ASTNode *n = ast_new_number(atoi(t.value));
        eat(p, TOKEN_NUMBER);
        return n;
    } else if (t.type == TOKEN_TRUE) {
        ASTNode *n = ast_new_bool(1);
        eat(p, TOKEN_TRUE);
        return n;
    } else if (t.type == TOKEN_FALSE) {
        ASTNode *n = ast_new_bool(0);
        eat(p, TOKEN_FALSE);
        return n;
    } else if (t.type == TOKEN_ID) {
        ASTNode *n = ast_new_variable(t.value);
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

static ASTNode *parse_arithmetic(Parser *p) {
    ASTNode *left = parse_primary(p);
    while (p->current_token.type == TOKEN_PLUS) {
        eat(p, TOKEN_PLUS);
        ASTNode *right = parse_primary(p);
        left = ast_new_bin_op("+", left, right);
    }
    return left;
}

static ASTNode *parse_comparison(Parser *p) {
    ASTNode *left = parse_arithmetic(p);
    while (p->current_token.type == TOKEN_EQ || p->current_token.type == TOKEN_NEQ ||
           p->current_token.type == TOKEN_LT || p->current_token.type == TOKEN_GT) {
        char *op = (p->current_token.type == TOKEN_EQ) ? "==" :
                   (p->current_token.type == TOKEN_NEQ) ? "!=" :
                   (p->current_token.type == TOKEN_LT) ? "<" : ">";
        eat(p, p->current_token.type);
        ASTNode *right = parse_arithmetic(p);
        left = ast_new_bin_op(op, left, right);
    }
    return left;
}

static ASTNode *parse_logical(Parser *p) {
    ASTNode *left = parse_comparison(p);
    while (p->current_token.type == TOKEN_AND || p->current_token.type == TOKEN_OR) {
        char *op = (p->current_token.type == TOKEN_AND) ? "&&" : "||";
        eat(p, p->current_token.type);
        ASTNode *right = parse_comparison(p);
        left = ast_new_bin_op(op, left, right);
    }
    return left;
}

static ASTNode *parse_expression(Parser *p) {
    return parse_logical(p);
}

static ASTNode *parse_statement(Parser *p) {
    if (p->current_token.type == TOKEN_INT || p->current_token.type == TOKEN_BOOL) {
        Type type = (p->current_token.type == TOKEN_INT) ? TYPE_INT : TYPE_BOOL;
        eat(p, p->current_token.type);
        char *name = strdup(p->current_token.value);
        eat(p, TOKEN_ID);
        eat(p, TOKEN_ASSIGN);
        ASTNode *val = parse_expression(p);
        eat(p, TOKEN_SEMICOLON);
        return ast_new_var_decl(type, name, val);
    } else if (p->current_token.type == TOKEN_PRINT) {
        eat(p, TOKEN_PRINT);
        eat(p, TOKEN_LPAREN);
        ASTNode *expr = parse_expression(p);
        eat(p, TOKEN_RPAREN);
        eat(p, TOKEN_SEMICOLON);
        return ast_new_print(expr);
    } else if (p->current_token.type == TOKEN_MATCH) {
        eat(p, TOKEN_MATCH);
        eat(p, TOKEN_LPAREN);
        ASTNode *expr = parse_expression(p);
        eat(p, TOKEN_RPAREN);
        eat(p, TOKEN_LBRACE);
        
        ASTNode *match_node = ast_new_match(expr);
        
        while (p->current_token.type == TOKEN_CASE) {
            eat(p, TOKEN_CASE);
            int val = atoi(p->current_token.value);
            eat(p, TOKEN_NUMBER);
            eat(p, TOKEN_COLON);
            ASTNode *stmt = parse_statement(p);
            ast_match_add_case(match_node, val, stmt);
        }
        
        if (p->current_token.type == TOKEN_DEFAULT) {
            eat(p, TOKEN_DEFAULT);
            eat(p, TOKEN_COLON);
            ASTNode *stmt = parse_statement(p);
            ast_match_set_default(match_node, stmt);
        }
        
        eat(p, TOKEN_RBRACE);
        return match_node;
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
