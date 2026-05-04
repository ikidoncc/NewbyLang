#include <stdlib.h>
#include <string.h>
#include "ast.h"

ASTNode *ast_new_number(int val) {
    ASTNode *node = malloc(sizeof(ASTNode));
    node->type = AST_NUMBER;
    node->data.number = val;
    return node;
}

ASTNode *ast_new_variable(char *name) {
    ASTNode *node = malloc(sizeof(ASTNode));
    node->type = AST_VARIABLE;
    node->data.var_name = strdup(name);
    return node;
}

ASTNode *ast_new_bin_op(char *op, ASTNode *left, ASTNode *right) {
    ASTNode *node = malloc(sizeof(ASTNode));
    node->type = AST_BIN_OP;
    node->data.bin_op.op = strdup(op);
    node->data.bin_op.left = left;
    node->data.bin_op.right = right;
    return node;
}

ASTNode *ast_new_var_decl(char *name, ASTNode *value) {
    ASTNode *node = malloc(sizeof(ASTNode));
    node->type = AST_VAR_DECL;
    node->data.var_decl.name = strdup(name);
    node->data.var_decl.value = value;
    return node;
}

ASTNode *ast_new_print(ASTNode *expr) {
    ASTNode *node = malloc(sizeof(ASTNode));
    node->type = AST_PRINT;
    node->data.print_expr = expr;
    return node;
}

ASTNode *ast_new_program() {
    ASTNode *node = malloc(sizeof(ASTNode));
    node->type = AST_PROGRAM;
    node->data.program.nodes = NULL;
    node->data.program.count = 0;
    return node;
}

void ast_program_add(ASTNode *program, ASTNode *node) {
    program->data.program.count++;
    program->data.program.nodes = realloc(program->data.program.nodes, sizeof(ASTNode*) * program->data.program.count);
    program->data.program.nodes[program->data.program.count - 1] = node;
}
