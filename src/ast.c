#include <stdlib.h>
#include <string.h>
#include "ast.h"

void ast_set_loc(ASTNode *node, int line, int col) {
    node->line = line;
    node->col = col;
}

ASTNode *ast_new_number(int val) {
    ASTNode *node = malloc(sizeof(ASTNode));
    node->type = AST_NUMBER;
    node->data.number = val;
    node->eval_type = TYPE_INT;
    ast_set_loc(node, 0, 0);
    return node;
}

ASTNode *ast_new_bool(int val) {
    ASTNode *node = malloc(sizeof(ASTNode));
    node->type = AST_BOOL;
    node->data.bool_val = val;
    node->eval_type = TYPE_BOOL;
    ast_set_loc(node, 0, 0);
    return node;
}

ASTNode *ast_new_float(double val) {
    ASTNode *node = malloc(sizeof(ASTNode));
    node->type = AST_FLOAT;
    node->data.float_val = val;
    node->eval_type = TYPE_FLOAT;
    ast_set_loc(node, 0, 0);
    return node;
}

ASTNode *ast_new_string(char *val) {
    ASTNode *node = malloc(sizeof(ASTNode));
    node->type = AST_STRING;
    node->data.string_val = strdup(val);
    node->eval_type = TYPE_STRING;
    ast_set_loc(node, 0, 0);
    return node;
}

ASTNode *ast_new_variable(char *name) {
    ASTNode *node = malloc(sizeof(ASTNode));
    node->type = AST_VARIABLE;
    node->data.var_name = strdup(name);
    node->eval_type = TYPE_UNKNOWN;
    ast_set_loc(node, 0, 0);
    return node;
}

ASTNode *ast_new_array_access(char *name, ASTNode *index) {
    ASTNode *node = malloc(sizeof(ASTNode));
    node->type = AST_ARRAY_ACCESS;
    node->data.array_access.name = strdup(name);
    node->data.array_access.index = index;
    node->eval_type = TYPE_UNKNOWN;
    ast_set_loc(node, 0, 0);
    return node;
}

ASTNode *ast_new_assign(char *name, ASTNode *value) {
    ASTNode *node = malloc(sizeof(ASTNode));
    node->type = AST_ASSIGN;
    node->data.assign.name = strdup(name);
    node->data.assign.value = value;
    node->eval_type = TYPE_UNKNOWN;
    ast_set_loc(node, 0, 0);
    return node;
}

ASTNode *ast_new_array_assign(char *name, ASTNode *index, ASTNode *value) {
    ASTNode *node = malloc(sizeof(ASTNode));
    node->type = AST_ARRAY_ASSIGN;
    node->data.array_assign.name = strdup(name);
    node->data.array_assign.index = index;
    node->data.array_assign.value = value;
    node->eval_type = TYPE_UNKNOWN;
    ast_set_loc(node, 0, 0);
    return node;
}

ASTNode *ast_new_bin_op(char *op, ASTNode *left, ASTNode *right) {
    ASTNode *node = malloc(sizeof(ASTNode));
    node->type = AST_BIN_OP;
    node->data.bin_op.op = strdup(op);
    node->data.bin_op.left = left;
    node->data.bin_op.right = right;
    node->eval_type = TYPE_UNKNOWN;
    ast_set_loc(node, 0, 0);
    return node;
}

ASTNode *ast_new_var_decl(Type type, char *name, ASTNode *value) {
    ASTNode *node = malloc(sizeof(ASTNode));
    node->type = AST_VAR_DECL;
    node->data.var_decl.type = type;
    node->data.var_decl.name = strdup(name);
    node->data.var_decl.value = value;
    node->eval_type = TYPE_UNKNOWN;
    ast_set_loc(node, 0, 0);
    return node;
}

ASTNode *ast_new_array_decl(Type type, char *name, int size) {
    ASTNode *node = malloc(sizeof(ASTNode));
    node->type = AST_ARRAY_DECL;
    node->data.array_decl.type = type;
    node->data.array_decl.name = strdup(name);
    node->data.array_decl.size = size;
    node->eval_type = TYPE_UNKNOWN;
    ast_set_loc(node, 0, 0);
    return node;
}

ASTNode *ast_new_print(ASTNode *expr) {
    ASTNode *node = malloc(sizeof(ASTNode));
    node->type = AST_PRINT;
    node->data.print_expr = expr;
    node->eval_type = TYPE_UNKNOWN;
    ast_set_loc(node, 0, 0);
    return node;
}

ASTNode *ast_new_program() {
    ASTNode *node = malloc(sizeof(ASTNode));
    node->type = AST_PROGRAM;
    node->data.program.nodes = NULL;
    node->data.program.count = 0;
    node->eval_type = TYPE_UNKNOWN;
    ast_set_loc(node, 0, 0);
    return node;
}

ASTNode *ast_new_if(ASTNode *condition, ASTNode *then_branch, ASTNode *else_branch) {
    ASTNode *node = malloc(sizeof(ASTNode));
    node->type = AST_IF;
    node->data.if_stmt.condition = condition;
    node->data.if_stmt.then_branch = then_branch;
    node->data.if_stmt.else_branch = else_branch;
    node->eval_type = TYPE_UNKNOWN;
    ast_set_loc(node, 0, 0);
    return node;
}

ASTNode *ast_new_while(ASTNode *condition, ASTNode *body) {
    ASTNode *node = malloc(sizeof(ASTNode));
    node->type = AST_WHILE;
    node->data.while_stmt.condition = condition;
    node->data.while_stmt.body = body;
    node->eval_type = TYPE_UNKNOWN;
    ast_set_loc(node, 0, 0);
    return node;
}

ASTNode *ast_new_match(ASTNode *expr) {
    ASTNode *node = malloc(sizeof(ASTNode));
    node->type = AST_MATCH;
    node->data.match.expr = expr;
    node->data.match.cases = NULL;
    node->data.match.case_count = 0;
    node->data.match.default_case = NULL;
    node->eval_type = TYPE_UNKNOWN;
    ast_set_loc(node, 0, 0);
    return node;
}

void ast_match_add_case(ASTNode *match, int val, ASTNode *stmt) {
    match->data.match.case_count++;
    match->data.match.cases = realloc(match->data.match.cases, sizeof(ASTNode*) * match->data.match.case_count);
    
    ASTNode *c = malloc(sizeof(ASTNode));
    c->type = AST_CASE;
    c->data.match_case.val = val;
    c->data.match_case.stmt = stmt;
    
    match->data.match.cases[match->data.match.case_count - 1] = c;
}

void ast_match_set_default(ASTNode *match, ASTNode *stmt) {
    match->data.match.default_case = stmt;
}

void ast_program_add(ASTNode *program, ASTNode *node) {
    program->data.program.count++;
    program->data.program.nodes = realloc(program->data.program.nodes, sizeof(ASTNode*) * program->data.program.count);
    program->data.program.nodes[program->data.program.count - 1] = node;
}
