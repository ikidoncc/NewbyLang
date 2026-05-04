#ifndef AST_H
#define AST_H

typedef enum {
    AST_VAR_DECL,
    AST_ASSIGN,
    AST_BIN_OP,
    AST_NUMBER,
    AST_VARIABLE,
    AST_PRINT,
    AST_PROGRAM
} ASTNodeType;

typedef struct ASTNode {
    ASTNodeType type;
    union {
        struct { char *name; struct ASTNode *value; } var_decl;
        struct { char *name; struct ASTNode *value; } assign;
        struct { char *op; struct ASTNode *left; struct ASTNode *right; } bin_op;
        int number;
        char *var_name;
        struct ASTNode *print_expr;
        struct { struct ASTNode **nodes; int count; } program;
    } data;
} ASTNode;

ASTNode *ast_new_number(int val);
ASTNode *ast_new_variable(char *name);
ASTNode *ast_new_bin_op(char *op, ASTNode *left, ASTNode *right);
ASTNode *ast_new_var_decl(char *name, ASTNode *value);
ASTNode *ast_new_print(ASTNode *expr);
ASTNode *ast_new_program();
void ast_program_add(ASTNode *program, ASTNode *node);

#endif
