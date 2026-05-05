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
        fprintf(stderr, "Syntax Error [%d:%d]: Unexpected token type %d, expected: %d\n", 
                p->current_token.line, p->current_token.col, p->current_token.type, type);
        exit(1);
    }
}

static ASTNode *parse_expression(Parser *p);

static ASTNode *parse_primary(Parser *p) {
    Token t = p->current_token;
    ASTNode *n = NULL;
    if (t.type == TOKEN_NUMBER) {
        n = ast_new_number(atoi(t.value));
        eat(p, TOKEN_NUMBER);
    } else if (t.type == TOKEN_FLOAT_LIT) {
        n = ast_new_float(atof(t.value));
        eat(p, TOKEN_FLOAT_LIT);
    } else if (t.type == TOKEN_STRING_LIT) {
        n = ast_new_string(t.value);
        eat(p, TOKEN_STRING_LIT);
    } else if (t.type == TOKEN_TRUE) {
        n = ast_new_bool(1);
        eat(p, TOKEN_TRUE);
    } else if (t.type == TOKEN_FALSE) {
        n = ast_new_bool(0);
        eat(p, TOKEN_FALSE);
    } else if (t.type == TOKEN_ID) {
        char *name = strdup(t.value);
        eat(p, TOKEN_ID);
        if (p->current_token.type == TOKEN_LBRACKET) {
            eat(p, TOKEN_LBRACKET);
            ASTNode *index = parse_expression(p);
            eat(p, TOKEN_RBRACKET);
            n = ast_new_array_access(name, index);
        } else if (p->current_token.type == TOKEN_LPAREN) {
            eat(p, TOKEN_LPAREN);
            n = ast_new_func_call(name);
            while (p->current_token.type != TOKEN_RPAREN) {
                ast_call_add_arg(n, parse_expression(p));
                if (p->current_token.type == TOKEN_COMMA) eat(p, TOKEN_COMMA); // Wait, TOKEN_COMMA not in lexer!
            }
            eat(p, TOKEN_RPAREN);
        } else {
            n = ast_new_variable(name);
        }
    } else if (t.type == TOKEN_LPAREN) {
        eat(p, TOKEN_LPAREN);
        n = parse_expression(p);
        eat(p, TOKEN_RPAREN);
    }
    if (n) ast_set_loc(n, t.line, t.col);
    return n;
}

static ASTNode *parse_term(Parser *p) {
    ASTNode *left = parse_primary(p);
    while (p->current_token.type == TOKEN_STAR || p->current_token.type == TOKEN_SLASH) {
        Token op_t = p->current_token;
        char *op = (op_t.type == TOKEN_STAR) ? "*" : "/";
        eat(p, op_t.type);
        ASTNode *right = parse_primary(p);
        left = ast_new_bin_op(op, left, right);
        ast_set_loc(left, op_t.line, op_t.col);
    }
    return left;
}

static ASTNode *parse_arithmetic(Parser *p) {
    ASTNode *left = parse_term(p);
    while (p->current_token.type == TOKEN_PLUS || p->current_token.type == TOKEN_MINUS) {
        Token op_t = p->current_token;
        char *op = (op_t.type == TOKEN_PLUS) ? "+" : "-";
        eat(p, op_t.type);
        ASTNode *right = parse_term(p);
        left = ast_new_bin_op(op, left, right);
        ast_set_loc(left, op_t.line, op_t.col);
    }
    return left;
}

static ASTNode *parse_comparison(Parser *p) {
    ASTNode *left = parse_arithmetic(p);
    while (p->current_token.type == TOKEN_EQ || p->current_token.type == TOKEN_NEQ ||
           p->current_token.type == TOKEN_LT || p->current_token.type == TOKEN_GT) {
        Token op_t = p->current_token;
        char *op = (p->current_token.type == TOKEN_EQ) ? "==" :
                   (p->current_token.type == TOKEN_NEQ) ? "!=" :
                   (p->current_token.type == TOKEN_LT) ? "<" : ">";
        eat(p, p->current_token.type);
        ASTNode *right = parse_arithmetic(p);
        left = ast_new_bin_op(op, left, right);
        ast_set_loc(left, op_t.line, op_t.col);
    }
    return left;
}

static ASTNode *parse_logical(Parser *p) {
    ASTNode *left = parse_comparison(p);
    while (p->current_token.type == TOKEN_AND || p->current_token.type == TOKEN_OR) {
        Token op_t = p->current_token;
        char *op = (p->current_token.type == TOKEN_AND) ? "&&" : "||";
        eat(p, p->current_token.type);
        ASTNode *right = parse_comparison(p);
        left = ast_new_bin_op(op, left, right);
        ast_set_loc(left, op_t.line, op_t.col);
    }
    return left;
}

static ASTNode *parse_expression(Parser *p) {
    return parse_logical(p);
}

static ASTNode *parse_statement(Parser *p) {
    Token t = p->current_token;
    if (t.type == TOKEN_INT || t.type == TOKEN_BOOL ||
        t.type == TOKEN_FLOAT || t.type == TOKEN_STRING ||
        t.type == TOKEN_INT8 || t.type == TOKEN_INT32 ||
        t.type == TOKEN_UINT || t.type == TOKEN_UINT8 ||
        t.type == TOKEN_UINT32) {
        
        Type type;
        switch (t.type) {
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
        eat(p, t.type);
        char *name = strdup(p->current_token.value);
        eat(p, TOKEN_ID);
        
        if (p->current_token.type == TOKEN_LBRACKET) {
            eat(p, TOKEN_LBRACKET);
            int size = atoi(p->current_token.value);
            eat(p, TOKEN_NUMBER);
            eat(p, TOKEN_RBRACKET);
            eat(p, TOKEN_SEMICOLON);
            ASTNode *node = ast_new_array_decl(type, name, size);
            ast_set_loc(node, t.line, t.col);
            return node;
        } else {
            eat(p, TOKEN_ASSIGN);
            ASTNode *val = parse_expression(p);
            eat(p, TOKEN_SEMICOLON);
            ASTNode *node = ast_new_var_decl(type, name, val);
            ast_set_loc(node, t.line, t.col);
            return node;
        }
    } else if (t.type == TOKEN_PRINT) {
        eat(p, TOKEN_PRINT);
        eat(p, TOKEN_LPAREN);
        ASTNode *expr = parse_expression(p);
        eat(p, TOKEN_RPAREN);
        eat(p, TOKEN_SEMICOLON);
        ASTNode *node = ast_new_print(expr);
        ast_set_loc(node, t.line, t.col);
        return node;
    } else if (t.type == TOKEN_IF) {
        eat(p, TOKEN_IF);
        eat(p, TOKEN_LPAREN);
        ASTNode *condition = parse_expression(p);
        eat(p, TOKEN_RPAREN);
        
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
        ASTNode *node = ast_new_if(condition, then_branch, else_branch);
        ast_set_loc(node, t.line, t.col);
        return node;
    } else if (t.type == TOKEN_WHILE) {
        eat(p, TOKEN_WHILE);
        eat(p, TOKEN_LPAREN);
        ASTNode *condition = parse_expression(p);
        eat(p, TOKEN_RPAREN);

        ASTNode *body = NULL;
        if (p->current_token.type == TOKEN_LBRACE) {
            eat(p, TOKEN_LBRACE);
            ASTNode *prog = ast_new_program();
            while (p->current_token.type != TOKEN_RBRACE && p->current_token.type != TOKEN_EOF) {
                ast_program_add(prog, parse_statement(p));
            }
            eat(p, TOKEN_RBRACE);
            body = prog;
        } else {
            body = parse_statement(p);
        }
        ASTNode *node = ast_new_while(condition, body);
        ast_set_loc(node, t.line, t.col);
        return node;
    } else if (t.type == TOKEN_FUNC) {
        eat(p, TOKEN_FUNC);
        Type ret_type = TYPE_INT; // Default
        if (p->current_token.type == TOKEN_INT || p->current_token.type == TOKEN_BOOL || p->current_token.type == TOKEN_STRING) {
             // Optional return type
             // For now keep it simple and just parse name
        }
        char *name = strdup(p->current_token.value);
        eat(p, TOKEN_ID);
        ASTNode *node = ast_new_func_decl(name, ret_type);
        eat(p, TOKEN_LPAREN);
        while (p->current_token.type != TOKEN_RPAREN) {
            Type p_type = TYPE_INT; // Simplified
            if (p->current_token.type == TOKEN_INT) eat(p, TOKEN_INT);
            else if (p->current_token.type == TOKEN_BOOL) { eat(p, TOKEN_BOOL); p_type = TYPE_BOOL; }
            else if (p->current_token.type == TOKEN_STRING) { eat(p, TOKEN_STRING); p_type = TYPE_STRING; }
            
            char *p_name = strdup(p->current_token.value);
            eat(p, TOKEN_ID);
            ast_func_add_param(node, p_type, p_name);
            if (p->current_token.type == TOKEN_COMMA) eat(p, TOKEN_COMMA);
        }
        eat(p, TOKEN_RPAREN);
        
        eat(p, TOKEN_LBRACE);
        ASTNode *body = ast_new_program();
        while (p->current_token.type != TOKEN_RBRACE && p->current_token.type != TOKEN_EOF) {
            ast_program_add(body, parse_statement(p));
        }
        eat(p, TOKEN_RBRACE);
        node->data.func_decl.body = body;
        ast_set_loc(node, t.line, t.col);
        return node;
    } else if (t.type == TOKEN_RETURN) {
        eat(p, TOKEN_RETURN);
        ASTNode *expr = parse_expression(p);
        eat(p, TOKEN_SEMICOLON);
        ASTNode *node = ast_new_return(expr);
        ast_set_loc(node, t.line, t.col);
        return node;
    } else if (t.type == TOKEN_MATCH) {
        eat(p, TOKEN_MATCH);
        eat(p, TOKEN_LPAREN);
        ASTNode *expr = parse_expression(p);
        eat(p, TOKEN_RPAREN);
        eat(p, TOKEN_LBRACE);
        
        ASTNode *match_node = ast_new_match(expr);
        ast_set_loc(match_node, t.line, t.col);
        
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
    } else if (t.type == TOKEN_ID) {
        char *name = strdup(t.value);
        eat(p, TOKEN_ID);
        if (p->current_token.type == TOKEN_LBRACKET) {
            eat(p, TOKEN_LBRACKET);
            ASTNode *index = parse_expression(p);
            eat(p, TOKEN_RBRACKET);
            eat(p, TOKEN_ASSIGN);
            ASTNode *val = parse_expression(p);
            eat(p, TOKEN_SEMICOLON);
            ASTNode *node = ast_new_array_assign(name, index, val);
            ast_set_loc(node, t.line, t.col);
            return node;
        } else if (p->current_token.type == TOKEN_ASSIGN) {
            eat(p, TOKEN_ASSIGN);
            ASTNode *val = parse_expression(p);
            eat(p, TOKEN_SEMICOLON);
            ASTNode *node = ast_new_assign(name, val);
            ast_set_loc(node, t.line, t.col);
            return node;
        } else if (p->current_token.type == TOKEN_LPAREN) {
            eat(p, TOKEN_LPAREN);
            ASTNode *node = ast_new_func_call(name);
            while (p->current_token.type != TOKEN_RPAREN) {
                ast_call_add_arg(node, parse_expression(p));
                if (p->current_token.type == TOKEN_COMMA) eat(p, TOKEN_COMMA);
            }
            eat(p, TOKEN_RPAREN);
            eat(p, TOKEN_SEMICOLON);
            ast_set_loc(node, t.line, t.col);
            return node;
        } else {
            fprintf(stderr, "Syntax Error [%d:%d]: Expected '=', '[' or '(' after variable name\n", 
                    p->current_token.line, p->current_token.col);
            exit(1);
        }
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
