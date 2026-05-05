#ifndef AST_H
#define AST_H

#include "types.h"

typedef enum {
    AST_VAR_DECL,
    AST_ARRAY_DECL,
    AST_ASSIGN,
    AST_ARRAY_ASSIGN,
    AST_DEREF_ASSIGN,
    AST_FUNC_DECL,
    AST_EXTERN_DECL,
    AST_SYSCALL,
    AST_FUNC_CALL,
    AST_RETURN,
    AST_ADDR_OF,
    AST_DEREF,
    AST_BIN_OP,
    AST_NUMBER,
    AST_BOOL,
    AST_FLOAT,
    AST_STRING,
    AST_VARIABLE,
    AST_ARRAY_ACCESS,
    AST_PRINT,
    AST_PROGRAM,
    AST_IF,
    AST_WHILE,
    AST_MATCH,
    AST_CASE
} ASTNodeType;

typedef struct ASTNode {
    ASTNodeType type;
    Type eval_type; // Type determined after semantic analysis
    int line;
    int col;
    union {
        struct { Type type; char *name; struct ASTNode *value; } var_decl;
        struct { Type type; char *name; int size; } array_decl;
        struct { char *name; struct ASTNode *value; } assign;
        struct { char *name; struct ASTNode *index; struct ASTNode *value; } array_assign;
        struct { struct ASTNode *ptr; struct ASTNode *value; } deref_assign;
        struct { char *name; Type return_type; struct { Type type; char *name; } params[8]; int param_count; struct ASTNode *body; } func_decl;
        struct { char *name; struct ASTNode *args[8]; int arg_count; } func_call;
        struct { struct ASTNode *args[7]; int arg_count; } syscall;
        struct { struct ASTNode *expr; } ret;
        struct { struct ASTNode *expr; } addr_of;
        struct { struct ASTNode *expr; } deref;
        struct { char *op; struct ASTNode *left; struct ASTNode *right; } bin_op;
        int number;
        int bool_val; // 0 or 1
        double float_val;
        char *string_val;
        char *var_name;
        struct { char *name; struct ASTNode *index; } array_access;
        struct ASTNode *print_expr;
        struct { struct ASTNode **nodes; int count; } program;
        struct { struct ASTNode *condition; struct ASTNode *then_branch; struct ASTNode *else_branch; } if_stmt;
        struct { struct ASTNode *condition; struct ASTNode *body; } while_stmt;
        struct { struct ASTNode *expr; struct ASTNode **cases; int case_count; struct ASTNode *default_case; } match;
        struct { int val; struct ASTNode *stmt; } match_case;
    } data;
} ASTNode;

ASTNode *ast_new_number(int val);
ASTNode *ast_new_bool(int val);
ASTNode *ast_new_float(double val);
ASTNode *ast_new_string(char *val);
ASTNode *ast_new_variable(char *name);
ASTNode *ast_new_array_access(char *name, ASTNode *index);
ASTNode *ast_new_assign(char *name, ASTNode *value);
ASTNode *ast_new_array_assign(char *name, ASTNode *index, ASTNode *value);
ASTNode *ast_new_func_decl(char *name, Type return_type);
ASTNode *ast_new_extern_decl(char *name, Type return_type);
ASTNode *ast_new_func_call(char *name);
ASTNode *ast_new_syscall();
ASTNode *ast_new_return(ASTNode *expr);
ASTNode *ast_new_addr_of(ASTNode *expr);
ASTNode *ast_new_deref(ASTNode *expr);
ASTNode *ast_new_bin_op(char *op, ASTNode *left, ASTNode *right);
ASTNode *ast_new_var_decl(Type type, char *name, ASTNode *value);
ASTNode *ast_new_array_decl(Type type, char *name, int size);
ASTNode *ast_new_print(ASTNode *expr);
ASTNode *ast_new_program();
ASTNode *ast_new_if(ASTNode *condition, ASTNode *then_branch, ASTNode *else_branch);
ASTNode *ast_new_while(ASTNode *condition, ASTNode *body);
ASTNode *ast_new_match(ASTNode *expr);
void ast_func_add_param(ASTNode *func, Type type, char *name);
void ast_call_add_arg(ASTNode *call, ASTNode *arg);
void ast_syscall_add_arg(ASTNode *syscall, ASTNode *arg);
void ast_match_add_case(ASTNode *match, int val, ASTNode *stmt);
void ast_match_set_default(ASTNode *match, ASTNode *stmt);
void ast_program_add(ASTNode *program, ASTNode *node);
void ast_set_loc(ASTNode *node, int line, int col);

#endif
