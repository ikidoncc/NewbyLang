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
    if (p->current_token.type == TOKEN_INT || p->current_token.type == TOKEN_BOOL ||
        p->current_token.type == TOKEN_FLOAT || p->current_token.type == TOKEN_STRING ||
        p->current_token.type == TOKEN_INT8 || p->current_token.type == TOKEN_INT32 ||
        p->current_token.type == TOKEN_UINT || p->current_token.type == TOKEN_UINT8 ||
        p->current_token.type == TOKEN_UINT32) {
        
        Type type;
        switch (p->current_token.type) {
            case TOKEN_INT: type = TYPE_INT; break;
            case TOKEN_BOOL: type = TYPE_BOOL; break;
            case TOKEN_FLOAT: type = TYPE_FLOAT; break;
            case TOKEN_STRING: type = TYPE_STRING; break;
            case TOKEN_INT8: type = TYPE_INT8; break;
            case TOKEN_INT32: type = TYPE_INT32; break;
            case TOKEN_UINT: type = TYPE_UINT; break;
            case TOKEN_UINT8: type = TYPE_UINT8; break;
            case TOKEN_UINT32: type = TYPE_UINT32; break;
            default: type = TYPE_UNKNOWN; break;
        }
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
    } else if (p->current_token.type == TOKEN_IF) {
        eat(p, TOKEN_IF);
        eat(p, TOKEN_LPAREN);
        ASTNode *condition = parse_expression(p);
        eat(p, TOKEN_RPAREN);
        
        // Suporte a blocos { } ou statement único
        ASTNode *then_branch = NULL;
        if (p->current_token.type == TOKEN_LBRACE) {
            eat(p, TOKEN_LBRACE);
            ASTNode *prog = ast_new_program();
            while (p->current_token.type != TOKEN_RBRACE && p->current_token.type != TOKEN_EOF) {
                ast_program_add(prog, parse_statement(p));
            }
            eat(p, TOKEN_RBRACE);
            then_branch = prog;
        } else {
            then_branch = parse_statement(p);
        }

        ASTNode *else_branch = NULL;
        if (p->current_token.type == TOKEN_ELSE) {
            eat(p, TOKEN_ELSE);
            if (p->current_token.type == TOKEN_LBRACE) {
                eat(p, TOKEN_LBRACE);
                ASTNode *prog = ast_new_program();
                while (p->current_token.type != TOKEN_RBRACE && p->current_token.type != TOKEN_EOF) {
                    ast_program_add(prog, parse_statement(p));
                }
                eat(p, TOKEN_RBRACE);
                else_branch = prog;
            } else {
                else_branch = parse_statement(p);
            }
        }
        return ast_new_if(condition, then_branch, else_branch);
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
