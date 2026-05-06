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
        fprintf(stderr, "Syntax Error [%d:%d]: Unexpected token type %d ('%s'), expected: %d\n", 
                p->current_token.line, p->current_token.col, p->current_token.type, 
                p->current_token.value ? p->current_token.value : "", type);
        exit(1);
    }
}

static ASTNode *parse_expression(Parser *p);

static Type parse_type(Parser *p, char **struct_name) {
    Token t = p->current_token;
    Type type = TYPE_INT;
    if (t.type == TOKEN_INT) { eat(p, TOKEN_INT); type = TYPE_INT; }
    else if (t.type == TOKEN_BOOL) { eat(p, TOKEN_BOOL); type = TYPE_BOOL; }
    else if (t.type == TOKEN_FLOAT) { eat(p, TOKEN_FLOAT); type = TYPE_FLOAT; }
    else if (t.type == TOKEN_STRING) { eat(p, TOKEN_STRING); type = TYPE_STRING; }
    else if (t.type == TOKEN_STRUCT) {
        eat(p, TOKEN_STRUCT);
        if (struct_name) *struct_name = strdup(p->current_token.value);
        eat(p, TOKEN_ID);
        type = TYPE_UNKNOWN;
    } else if (t.type == TOKEN_ID) {
        if (struct_name) *struct_name = strdup(t.value);
        eat(p, TOKEN_ID);
        type = TYPE_UNKNOWN;
    }
    while (p->current_token.type == TOKEN_STAR) {
        eat(p, TOKEN_STAR);
        type = TYPE_PTR;
    }
    return type;
}

static ASTNode *parse_atom(Parser *p) {
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
    } else if (t.type == TOKEN_SIZEOF) {
        eat(p, TOKEN_SIZEOF);
        eat(p, TOKEN_LPAREN);
        char *s_name = NULL;
        Type type = parse_type(p, &s_name);
        eat(p, TOKEN_RPAREN);
        n = ast_new_sizeof(type, s_name);
    } else if (t.type == TOKEN_SELF) {
        eat(p, TOKEN_SELF);
        n = ast_new_self();
    } else if (t.type == TOKEN_ID) {
        char *name = strdup(t.value);
        eat(p, TOKEN_ID);
        n = ast_new_variable(name);
    } else if (t.type == TOKEN_SYSCALL) {
        eat(p, TOKEN_SYSCALL);
        eat(p, TOKEN_LPAREN);
        n = ast_new_syscall();
        while (p->current_token.type != TOKEN_RPAREN) {
            ast_syscall_add_arg(n, parse_expression(p));
            if (p->current_token.type == TOKEN_COMMA) eat(p, TOKEN_COMMA);
        }
        eat(p, TOKEN_RPAREN);
    } else if (t.type == TOKEN_LPAREN) {
        eat(p, TOKEN_LPAREN);
        n = parse_expression(p);
        eat(p, TOKEN_RPAREN);
    }
    if (n) ast_set_loc(n, t.line, t.col);
    return n;
}

static ASTNode *parse_postfix(Parser *p) {
    ASTNode *n = parse_atom(p);
    while (p->current_token.type == TOKEN_LBRACKET || p->current_token.type == TOKEN_DOT || p->current_token.type == TOKEN_LPAREN) {
        Token t = p->current_token;
        if (t.type == TOKEN_LBRACKET) {
            eat(p, TOKEN_LBRACKET);
            ASTNode *index = parse_expression(p);
            eat(p, TOKEN_RBRACKET);
            if (n->type == AST_VARIABLE) {
                char *name = n->data.var_name;
                n = ast_new_array_access(name, index);
            }
        } else if (t.type == TOKEN_DOT) {
            eat(p, TOKEN_DOT);
            char *member = strdup(p->current_token.value);
            eat(p, TOKEN_ID);
            if (p->current_token.type == TOKEN_LPAREN) {
                eat(p, TOKEN_LPAREN);
                ASTNode *call = ast_new_func_call(member);
                call->data.func_call.obj = n;
                while (p->current_token.type != TOKEN_RPAREN) {
                    ast_call_add_arg(call, parse_expression(p));
                    if (p->current_token.type == TOKEN_COMMA) eat(p, TOKEN_COMMA);
                }
                eat(p, TOKEN_RPAREN);
                n = call;
            } else {
                n = ast_new_member_access(n, member);
            }
        } else if (t.type == TOKEN_LPAREN) {
            eat(p, TOKEN_LPAREN);
            if (n->type == AST_VARIABLE) {
                char *name = strdup(n->data.var_name);
                n = ast_new_func_call(name);
            }
            while (p->current_token.type != TOKEN_RPAREN) {
                ast_call_add_arg(n, parse_expression(p));
                if (p->current_token.type == TOKEN_COMMA) eat(p, TOKEN_COMMA);
            }
            eat(p, TOKEN_RPAREN);
        }
        ast_set_loc(n, t.line, t.col);
    }
    return n;
}

static ASTNode *parse_unary(Parser *p) {
    if (p->current_token.type == TOKEN_STAR) {
        Token t = p->current_token;
        eat(p, TOKEN_STAR);
        ASTNode *node = ast_new_deref(parse_unary(p));
        ast_set_loc(node, t.line, t.col);
        return node;
    } else if (p->current_token.type == TOKEN_AMPERSAND) {
        Token t = p->current_token;
        eat(p, TOKEN_AMPERSAND);
        ASTNode *node = ast_new_addr_of(parse_unary(p));
        ast_set_loc(node, t.line, t.col);
        return node;
    }
    return parse_postfix(p);
}

static ASTNode *parse_term(Parser *p) {
    ASTNode *left = parse_unary(p);
    while (p->current_token.type == TOKEN_STAR || p->current_token.type == TOKEN_SLASH) {
        Token op_t = p->current_token;
        char *op = (op_t.type == TOKEN_STAR) ? "*" : "/";
        eat(p, op_t.type);
        ASTNode *right = parse_unary(p);
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
    int is_pub = 0;
    if (t.type == TOKEN_PUB) {
        is_pub = 1;
        eat(p, TOKEN_PUB);
        t = p->current_token;
    }

    if (t.type == TOKEN_IMPORT) {
        eat(p, TOKEN_IMPORT);
        char *mod = strdup(p->current_token.value);
        eat(p, TOKEN_STRING_LIT);
        eat(p, TOKEN_SEMICOLON);
        ASTNode *node = ast_new_import(mod);
        ast_set_loc(node, t.line, t.col);
        return node;
    } else if (t.type == TOKEN_ENUM) {
        eat(p, TOKEN_ENUM);
        char *name = strdup(p->current_token.value);
        eat(p, TOKEN_ID);
        eat(p, TOKEN_LBRACE);
        ASTNode *node = ast_new_enum_def(name);
        while (p->current_token.type != TOKEN_RBRACE) {
            char *v_name = strdup(p->current_token.value);
            eat(p, TOKEN_ID);
            ast_enum_add_variant(node, v_name);
            if (p->current_token.type == TOKEN_COMMA) eat(p, TOKEN_COMMA);
        }
        eat(p, TOKEN_RBRACE);
        eat(p, TOKEN_SEMICOLON);
        ast_set_loc(node, t.line, t.col);
        return node;
    } else if (t.type == TOKEN_STRUCT) {
        eat(p, TOKEN_STRUCT);
        char *name = strdup(p->current_token.value);
        eat(p, TOKEN_ID);
        if (p->current_token.type == TOKEN_LBRACE) {
            eat(p, TOKEN_LBRACE);
            ASTNode *node = ast_new_struct_def(name);
            while (p->current_token.type != TOKEN_RBRACE) {
                if (p->current_token.type == TOKEN_FUNC) {
                    ASTNode *method = parse_statement(p);
                    ast_struct_add_method(node, method);
                } else {
                    char *s_name = NULL;
                    Type m_type = parse_type(p, &s_name);
                    char *m_name = strdup(p->current_token.value);
                    eat(p, TOKEN_ID);
                    eat(p, TOKEN_SEMICOLON);
                    ast_struct_add_member(node, m_type, m_name);
                }
            }
            eat(p, TOKEN_RBRACE);
            eat(p, TOKEN_SEMICOLON);
            ast_set_loc(node, t.line, t.col);
            return node;
        } else {
            char *var_name = strdup(p->current_token.value);
            eat(p, TOKEN_ID);
            eat(p, TOKEN_SEMICOLON);
            ASTNode *node = ast_new_var_decl(TYPE_UNKNOWN, var_name, NULL);
            node->data.var_decl.struct_name = name;
            ast_set_loc(node, t.line, t.col);
            return node;
        }
    } else if (t.type == TOKEN_INT || t.type == TOKEN_BOOL ||
        t.type == TOKEN_FLOAT || t.type == TOKEN_STRING ||
        t.type == TOKEN_INT8 || t.type == TOKEN_INT32 ||
        t.type == TOKEN_UINT || t.type == TOKEN_UINT8 ||
        t.type == TOKEN_UINT32 || t.type == TOKEN_ID) {
        
        Type type = TYPE_INT;
        char *struct_name = NULL;
        if (t.type == TOKEN_ID) {
            Token next = lexer_peek(p->lexer);
            if (next.type == TOKEN_ID || next.type == TOKEN_STAR) {
                struct_name = strdup(t.value);
                type = TYPE_UNKNOWN;
                eat(p, TOKEN_ID);
            } else goto handle_id_stmt;
        } else {
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
        }
        if (p->current_token.type == TOKEN_STAR) {
            eat(p, TOKEN_STAR);
            type = TYPE_PTR;
        }
        char *name = strdup(p->current_token.value);
        eat(p, TOKEN_ID);
        if (p->current_token.type == TOKEN_LBRACKET) {
            eat(p, TOKEN_LBRACKET);
            int size = atoi(p->current_token.value);
            eat(p, TOKEN_NUMBER);
            eat(p, TOKEN_RBRACKET);
            eat(p, TOKEN_SEMICOLON);
            ASTNode *node = ast_new_array_decl(type, name, size);
            node->data.array_decl.is_pub = is_pub;
            ast_set_loc(node, t.line, t.col);
            return node;
        } else {
            ASTNode *val = NULL;
            if (p->current_token.type == TOKEN_ASSIGN) {
                eat(p, TOKEN_ASSIGN);
                val = parse_expression(p);
            }
            eat(p, TOKEN_SEMICOLON);
            ASTNode *node = ast_new_var_decl(type, name, val);
            node->data.var_decl.is_pub = is_pub;
            node->data.var_decl.struct_name = struct_name;
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
            while (p->current_token.type != TOKEN_RBRACE && p->current_token.type != TOKEN_EOF) ast_program_add(prog, parse_statement(p));
            eat(p, TOKEN_RBRACE);
            then_branch = prog;
        } else then_branch = parse_statement(p);
        ASTNode *else_branch = NULL;
        if (p->current_token.type == TOKEN_ELSE) {
            eat(p, TOKEN_ELSE);
            if (p->current_token.type == TOKEN_LBRACE) {
                eat(p, TOKEN_LBRACE);
                ASTNode *prog = ast_new_program();
                while (p->current_token.type != TOKEN_RBRACE && p->current_token.type != TOKEN_EOF) ast_program_add(prog, parse_statement(p));
                eat(p, TOKEN_RBRACE);
                else_branch = prog;
            } else else_branch = parse_statement(p);
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
            while (p->current_token.type != TOKEN_RBRACE && p->current_token.type != TOKEN_EOF) ast_program_add(prog, parse_statement(p));
            eat(p, TOKEN_RBRACE);
            body = prog;
        } else body = parse_statement(p);
        ASTNode *node = ast_new_while(condition, body);
        ast_set_loc(node, t.line, t.col);
        return node;
    } else if (t.type == TOKEN_FUNC) {
        eat(p, TOKEN_FUNC);
        char *name = strdup(p->current_token.value);
        eat(p, TOKEN_ID);
        ASTNode *node = ast_new_func_decl(name, TYPE_INT);
        node->data.func_decl.is_pub = is_pub;
        eat(p, TOKEN_LPAREN);
        while (p->current_token.type != TOKEN_RPAREN) {
            char *s_name = NULL;
            Type p_type = parse_type(p, &s_name);
            char *p_name = strdup(p->current_token.value);
            eat(p, TOKEN_ID);
            ast_func_add_param(node, p_type, p_name);
            if (p->current_token.type == TOKEN_COMMA) eat(p, TOKEN_COMMA);
        }
        eat(p, TOKEN_RPAREN);
        if (p->current_token.type == TOKEN_COLON) {
            eat(p, TOKEN_COLON);
            char *s_name = NULL;
            node->data.func_decl.return_type = parse_type(p, &s_name);
        }
        eat(p, TOKEN_LBRACE);
        ASTNode *body = ast_new_program();
        while (p->current_token.type != TOKEN_RBRACE && p->current_token.type != TOKEN_EOF) ast_program_add(body, parse_statement(p));
        eat(p, TOKEN_RBRACE);
        node->data.func_decl.body = body;
        ast_set_loc(node, t.line, t.col);
        return node;
    } else if (t.type == TOKEN_EXTERN) {
        eat(p, TOKEN_EXTERN);
        eat(p, TOKEN_FUNC);
        char *name = strdup(p->current_token.value);
        eat(p, TOKEN_ID);
        ASTNode *node = ast_new_extern_decl(name, TYPE_INT);
        eat(p, TOKEN_LPAREN);
        while (p->current_token.type != TOKEN_RPAREN) {
            char *s_name = NULL;
            Type p_type = parse_type(p, &s_name);
            char *p_name = strdup(p->current_token.value);
            eat(p, TOKEN_ID);
            ast_func_add_param(node, p_type, p_name);
            if (p->current_token.type == TOKEN_COMMA) eat(p, TOKEN_COMMA);
        }
        eat(p, TOKEN_RPAREN);
        if (p->current_token.type == TOKEN_COLON) {
            eat(p, TOKEN_COLON);
            char *s_name = NULL;
            node->data.func_decl.return_type = parse_type(p, &s_name);
        }
        eat(p, TOKEN_SEMICOLON);
        ast_set_loc(node, t.line, t.col);
        return node;
    } else if (t.type == TOKEN_SYSCALL) {
        ASTNode *node = parse_expression(p);
        eat(p, TOKEN_SEMICOLON);
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
            ASTNode *case_val = parse_expression(p);
            eat(p, TOKEN_COLON);
            ASTNode *stmt = parse_statement(p);
            ast_match_add_case(match_node, case_val, stmt);
        }
        if (p->current_token.type == TOKEN_DEFAULT) {
            eat(p, TOKEN_DEFAULT);
            eat(p, TOKEN_COLON);
            ast_match_set_default(match_node, parse_statement(p));
        }
        eat(p, TOKEN_RBRACE);
        return match_node;
    } else if (t.type == TOKEN_LPAREN || t.type == TOKEN_STAR || t.type == TOKEN_AMPERSAND || t.type == TOKEN_SELF) {
        ASTNode *expr = parse_expression(p);
        if (p->current_token.type == TOKEN_ASSIGN) {
            eat(p, TOKEN_ASSIGN);
            ASTNode *val = parse_expression(p);
            eat(p, TOKEN_SEMICOLON);
            if (expr->type == AST_MEMBER_ACCESS) {
                ASTNode *node = malloc(sizeof(ASTNode));
                node->type = AST_MEMBER_ASSIGN;
                node->data.member_assign.obj = expr->data.member_access.ptr;
                node->data.member_assign.member = expr->data.member_access.member;
                node->data.member_assign.value = val;
                ast_set_loc(node, t.line, t.col);
                return node;
            } else if (expr->type == AST_DEREF) {
                ASTNode *node = malloc(sizeof(ASTNode));
                node->type = AST_DEREF_ASSIGN;
                node->data.deref_assign.ptr = expr->data.deref.expr;
                node->data.deref_assign.value = val;
                ast_set_loc(node, t.line, t.col);
                return node;
            }
        }
        eat(p, TOKEN_SEMICOLON);
        return expr;
    } else if (t.type == TOKEN_ID) {
handle_id_stmt:;
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
        } else if (p->current_token.type == TOKEN_DOT) {
            eat(p, TOKEN_DOT);
            char *member = strdup(p->current_token.value);
            eat(p, TOKEN_ID);
            if (p->current_token.type == TOKEN_LPAREN) {
                eat(p, TOKEN_LPAREN);
                ASTNode *node = ast_new_func_call(member);
                node->data.func_call.obj = ast_new_variable(name);
                while (p->current_token.type != TOKEN_RPAREN) {
                    ast_call_add_arg(node, parse_expression(p));
                    if (p->current_token.type == TOKEN_COMMA) eat(p, TOKEN_COMMA);
                }
                eat(p, TOKEN_RPAREN);
                eat(p, TOKEN_SEMICOLON);
                ast_set_loc(node, t.line, t.col);
                return node;
            } else {
                eat(p, TOKEN_ASSIGN);
                ASTNode *val = parse_expression(p);
                eat(p, TOKEN_SEMICOLON);
                ASTNode *node = malloc(sizeof(ASTNode));
                node->type = AST_MEMBER_ASSIGN;
                node->data.member_assign.obj = ast_new_variable(name);
                node->data.member_assign.member = member;
                node->data.member_assign.value = val;
                ast_set_loc(node, t.line, t.col);
                return node;
            }
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
        }
    }
    fprintf(stderr, "Syntax Error [%d:%d]: Unexpected token %d ('%s')\n", t.line, t.col, t.type, t.value ? t.value : "");
    exit(1);
}

ASTNode *parser_parse(Parser *p) {
    ASTNode *prog = ast_new_program();
    while (p->current_token.type != TOKEN_EOF) {
        ASTNode *stmt = parse_statement(p);
        if (stmt) ast_program_add(prog, stmt);
    }
    return prog;
}
