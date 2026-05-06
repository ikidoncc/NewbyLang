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

ASTNode *ast_new_func_decl(char *name, Type return_type) {
    ASTNode *node = malloc(sizeof(ASTNode));
    node->type = AST_FUNC_DECL;
    node->data.func_decl.name = strdup(name);
    node->data.func_decl.return_type = return_type;
    node->data.func_decl.return_struct_name = NULL;
    node->data.func_decl.param_count = 0;
    node->data.func_decl.body = NULL;
    node->data.func_decl.is_pub = 0;
    node->data.func_decl.parent_struct = NULL;
    node->eval_type = TYPE_UNKNOWN;
    ast_set_loc(node, 0, 0);
    return node;
}

ASTNode *ast_new_extern_decl(char *name, Type return_type) {
    ASTNode *node = malloc(sizeof(ASTNode));
    node->type = AST_EXTERN_DECL;
    node->data.func_decl.name = strdup(name);
    node->data.func_decl.return_type = return_type;
    node->data.func_decl.return_struct_name = NULL;
    node->data.func_decl.param_count = 0;
    node->data.func_decl.body = NULL;
    node->eval_type = TYPE_UNKNOWN;
    ast_set_loc(node, 0, 0);
    return node;
}

ASTNode *ast_new_struct_def(char *name) {
    ASTNode *node = malloc(sizeof(ASTNode));
    node->type = AST_STRUCT_DEF;
    node->data.struct_def.name = strdup(name);
    node->data.struct_def.member_count = 0;
    node->data.struct_def.method_count = 0;
    node->eval_type = TYPE_UNKNOWN;
    ast_set_loc(node, 0, 0);
    return node;
}

ASTNode *ast_new_enum_def(char *name) {
    ASTNode *node = malloc(sizeof(ASTNode));
    node->type = AST_ENUM_DEF;
    node->data.enum_def.name = strdup(name);
    node->data.enum_def.variant_count = 0;
    node->data.enum_def.generic_count = 0;
    node->eval_type = TYPE_UNKNOWN;
    ast_set_loc(node, 0, 0);
    return node;
}

void ast_enum_add_variant(ASTNode *e, char *name, Type type, char *struct_name) {
    int i = e->data.enum_def.variant_count++;
    e->data.enum_def.variants[i].name = strdup(name);
    e->data.enum_def.variants[i].type = type;
    e->data.enum_def.variants[i].struct_name = struct_name ? strdup(struct_name) : NULL;
}

void ast_enum_add_generic(ASTNode *e, char *name) {
    int i = e->data.enum_def.generic_count++;
    e->data.enum_def.generic_params[i] = strdup(name);
}

ASTNode *ast_new_sizeof(Type type, char *struct_name) {
    ASTNode *node = malloc(sizeof(ASTNode));
    node->type = AST_SIZEOF;
    node->data.size_of.type = type;
    node->data.size_of.struct_name = struct_name ? strdup(struct_name) : NULL;
    node->eval_type = TYPE_INT;
    ast_set_loc(node, 0, 0);
    return node;
}

ASTNode *ast_new_import(char *module) {
    ASTNode *node = malloc(sizeof(ASTNode));
    node->type = AST_IMPORT;
    node->data.import.module_name = strdup(module);
    node->eval_type = TYPE_UNKNOWN;
    ast_set_loc(node, 0, 0);
    return node;
}

ASTNode *ast_new_ns_access(char *module, char *name) {
    ASTNode *node = malloc(sizeof(ASTNode));
    node->type = AST_NS_ACCESS;
    node->data.ns_access.module = strdup(module);
    node->data.ns_access.name = strdup(name);
    node->eval_type = TYPE_UNKNOWN;
    ast_set_loc(node, 0, 0);
    return node;
}

ASTNode *ast_new_member_access(ASTNode *ptr, char *member) {
    ASTNode *node = malloc(sizeof(ASTNode));
    node->type = AST_MEMBER_ACCESS;
    node->data.member_access.ptr = ptr;
    node->data.member_access.member = strdup(member);
    node->eval_type = TYPE_UNKNOWN;
    ast_set_loc(node, 0, 0);
    return node;
}

ASTNode *ast_new_func_call(char *name) {
    ASTNode *node = malloc(sizeof(ASTNode));
    node->type = AST_FUNC_CALL;
    node->data.func_call.name = strdup(name);
    node->data.func_call.arg_count = 0;
    node->data.func_call.obj = NULL;
    node->eval_type = TYPE_UNKNOWN;
    ast_set_loc(node, 0, 0);
    return node;
}

ASTNode *ast_new_syscall() {
    ASTNode *node = malloc(sizeof(ASTNode));
    node->type = AST_SYSCALL;
    node->data.syscall.arg_count = 0;
    node->eval_type = TYPE_INT;
    ast_set_loc(node, 0, 0);
    return node;
}

ASTNode *ast_new_try(ASTNode *expr) {
    ASTNode *node = malloc(sizeof(ASTNode));
    node->type = AST_TRY;
    node->data.try.expr = expr;
    node->eval_type = TYPE_UNKNOWN;
    ast_set_loc(node, 0, 0);
    return node;
}

ASTNode *ast_new_return(ASTNode *expr) {
    ASTNode *node = malloc(sizeof(ASTNode));
    node->type = AST_RETURN;
    node->data.ret.expr = expr;
    node->eval_type = TYPE_UNKNOWN;
    ast_set_loc(node, 0, 0);
    return node;
}

ASTNode *ast_new_addr_of(ASTNode *expr) {
    ASTNode *node = malloc(sizeof(ASTNode));
    node->type = AST_ADDR_OF;
    node->data.addr_of.expr = expr;
    node->eval_type = TYPE_PTR;
    ast_set_loc(node, 0, 0);
    return node;
}

ASTNode *ast_new_deref(ASTNode *expr) {
    ASTNode *node = malloc(sizeof(ASTNode));
    node->type = AST_DEREF;
    node->data.deref.expr = expr;
    node->eval_type = TYPE_UNKNOWN;
    ast_set_loc(node, 0, 0);
    return node;
}

void ast_func_add_param(ASTNode *func, Type type, char *name, char *struct_name) {
    int i = func->data.func_decl.param_count++;
    func->data.func_decl.params[i].type = type;
    func->data.func_decl.params[i].name = strdup(name);
    func->data.func_decl.params[i].struct_name = struct_name ? strdup(struct_name) : NULL;
}

void ast_call_add_arg(ASTNode *call, ASTNode *arg) {
    int i = call->data.func_call.arg_count++;
    call->data.func_call.args[i] = arg;
}

void ast_syscall_add_arg(ASTNode *syscall, ASTNode *arg) {
    int i = syscall->data.syscall.arg_count++;
    syscall->data.syscall.args[i] = arg;
}

void ast_struct_add_member(ASTNode *s, Type type, char *name) {
    int i = s->data.struct_def.member_count++;
    s->data.struct_def.members[i].type = type;
    s->data.struct_def.members[i].name = strdup(name);
}

void ast_struct_add_method(ASTNode *s, ASTNode *method) {
    int i = s->data.struct_def.method_count++;
    s->data.struct_def.methods[i] = method;
    method->data.func_decl.parent_struct = strdup(s->data.struct_def.name);
}

ASTNode *ast_new_self() {
    ASTNode *node = malloc(sizeof(ASTNode));
    node->type = AST_SELF;
    node->eval_type = TYPE_PTR;
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
    node->data.var_decl.is_pub = 0;
    node->data.var_decl.struct_name = NULL;
    node->data.var_decl.enum_name = NULL;
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
    node->data.array_decl.is_pub = 0;
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

void ast_match_add_case(ASTNode *match, ASTNode *val_node, char *capture_var, ASTNode *stmt) {
    match->data.match.case_count++;
    match->data.match.cases = realloc(match->data.match.cases, sizeof(ASTNode*) * match->data.match.case_count);
    ASTNode *c = malloc(sizeof(ASTNode));
    c->type = AST_CASE;
    c->data.match_case.val_node = val_node;
    c->data.match_case.capture_var = capture_var ? strdup(capture_var) : NULL;
    c->data.match_case.val = 0;
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

void ast_free(ASTNode *node) {
    if (!node) return;
    switch (node->type) {
        case AST_VAR_DECL:
            free(node->data.var_decl.name);
            if (node->data.var_decl.value) ast_free(node->data.var_decl.value);
            if (node->data.var_decl.struct_name) free(node->data.var_decl.struct_name);
            if (node->data.var_decl.enum_name) free(node->data.var_decl.enum_name);
            break;
        case AST_ARRAY_DECL:
            free(node->data.array_decl.name);
            break;
        case AST_ASSIGN:
            free(node->data.assign.name);
            ast_free(node->data.assign.value);
            break;
        case AST_ARRAY_ASSIGN:
            free(node->data.array_assign.name);
            ast_free(node->data.array_assign.index);
            ast_free(node->data.array_assign.value);
            break;
        case AST_DEREF_ASSIGN:
            ast_free(node->data.deref_assign.ptr);
            ast_free(node->data.deref_assign.value);
            break;
        case AST_MEMBER_ASSIGN:
            ast_free(node->data.member_assign.obj);
            free(node->data.member_assign.member);
            ast_free(node->data.member_assign.value);
            break;
        case AST_FUNC_DECL:
        case AST_EXTERN_DECL:
            free(node->data.func_decl.name);
            if (node->data.func_decl.return_struct_name) free(node->data.func_decl.return_struct_name);
            for (int i = 0; i < node->data.func_decl.param_count; i++) {
                free(node->data.func_decl.params[i].name);
                if (node->data.func_decl.params[i].struct_name) free(node->data.func_decl.params[i].struct_name);
            }
            if (node->data.func_decl.body) ast_free(node->data.func_decl.body);
            if (node->data.func_decl.parent_struct) free(node->data.func_decl.parent_struct);
            break;
        case AST_FUNC_CALL:
            free(node->data.func_call.name);
            for (int i = 0; i < node->data.func_call.arg_count; i++) ast_free(node->data.func_call.args[i]);
            if (node->data.func_call.obj) ast_free(node->data.func_call.obj);
            break;
        case AST_STRUCT_DEF:
            free(node->data.struct_def.name);
            for (int i = 0; i < node->data.struct_def.member_count; i++) {
                free(node->data.struct_def.members[i].name);
            }
            for (int i = 0; i < node->data.struct_def.method_count; i++) {
                ast_free(node->data.struct_def.methods[i]);
            }
            break;
        case AST_ENUM_DEF:
            free(node->data.enum_def.name);
            for (int i = 0; i < node->data.enum_def.variant_count; i++) {
                free(node->data.enum_def.variants[i].name);
                if (node->data.enum_def.variants[i].struct_name) free(node->data.enum_def.variants[i].struct_name);
            }
            for (int i = 0; i < node->data.enum_def.generic_count; i++) {
                free(node->data.enum_def.generic_params[i]);
            }
            break;
        case AST_MEMBER_ACCESS:
            ast_free(node->data.member_access.ptr);
            free(node->data.member_access.member);
            break;
        case AST_SIZEOF:
            if (node->data.size_of.struct_name) free(node->data.size_of.struct_name);
            break;
        case AST_NS_ACCESS:
            free(node->data.ns_access.module);
            free(node->data.ns_access.name);
            break;
        case AST_IMPORT:
            free(node->data.import.module_name);
            break;
        case AST_SYSCALL:
            for (int i = 0; i < node->data.syscall.arg_count; i++) ast_free(node->data.syscall.args[i]);
            break;
        case AST_TRY:
            ast_free(node->data.try.expr);
            break;
        case AST_RETURN:
            ast_free(node->data.ret.expr);
            break;
        case AST_ADDR_OF:
            ast_free(node->data.addr_of.expr);
            break;
        case AST_DEREF:
            ast_free(node->data.deref.expr);
            break;
        case AST_BIN_OP:
            ast_free(node->data.bin_op.left);
            ast_free(node->data.bin_op.right);
            break;
        case AST_VARIABLE:
            free(node->data.var_name);
            break;
        case AST_ARRAY_ACCESS:
            free(node->data.array_access.name);
            ast_free(node->data.array_access.index);
            break;
        case AST_PRINT:
            ast_free(node->data.print_expr);
            break;
        case AST_PROGRAM:
            for (int i = 0; i < node->data.program.count; i++) ast_free(node->data.program.nodes[i]);
            free(node->data.program.nodes);
            break;
        case AST_IF:
            ast_free(node->data.if_stmt.condition);
            ast_free(node->data.if_stmt.then_branch);
            if (node->data.if_stmt.else_branch) ast_free(node->data.if_stmt.else_branch);
            break;
        case AST_WHILE:
            ast_free(node->data.while_stmt.condition);
            ast_free(node->data.while_stmt.body);
            break;
        case AST_MATCH:
            ast_free(node->data.match.expr);
            for (int i = 0; i < node->data.match.case_count; i++) ast_free(node->data.match.cases[i]);
            free(node->data.match.cases);
            if (node->data.match.default_case) ast_free(node->data.match.default_case);
            break;
        case AST_CASE:
            ast_free(node->data.match_case.val_node);
            if (node->data.match_case.capture_var) free(node->data.match_case.capture_var);
            ast_free(node->data.match_case.stmt);
            break;
        default: break;
    }
    free(node);
}
