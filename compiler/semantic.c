#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "semantic.h"

static int is_integer_type(Type t) {
    return t == TYPE_INT || t == TYPE_INT8 || t == TYPE_INT32 || 
           t == TYPE_UINT || t == TYPE_UINT8 || t == TYPE_UINT32;
}

typedef struct StructDef {
    char *name;
    struct { char *name; Type type; } members[16];
    int member_count;
} StructDef;

static StructDef global_structs[16];
static int global_struct_count = 0;

static StructDef *find_struct_def(const char *name) {
    for (int i = 0; i < global_struct_count; i++) {
        if (strcmp(global_structs[i].name, name) == 0) return &global_structs[i];
    }
    return NULL;
}

void semantic_analyze(ASTNode *node, SymbolTable *tab) {
    if (!node) return;

    switch (node->type) {
        case AST_PROGRAM: {
            SymbolTable *new_tab = symtab_new(tab);
            for (int i = 0; i < node->data.program.count; i++) {
                semantic_analyze(node->data.program.nodes[i], new_tab);
            }
            break;
        }

        case AST_STRUCT_DEF: {
            StructDef *sd = &global_structs[global_struct_count++];
            sd->name = strdup(node->data.struct_def.name);
            sd->member_count = node->data.struct_def.member_count;
            for (int i = 0; i < sd->member_count; i++) {
                sd->members[i].name = strdup(node->data.struct_def.members[i].name);
                sd->members[i].type = node->data.struct_def.members[i].type;
            }
            break;
        }

        case AST_VAR_DECL:
            if (node->data.var_decl.value) semantic_analyze(node->data.var_decl.value, tab);
            symtab_add(tab, node->data.var_decl.name, 0, node->data.var_decl.type, 0, 0);
            if (node->data.var_decl.struct_name) {
                Symbol *s = symtab_lookup(tab, node->data.var_decl.name);
                // Tag symbol as struct instance somehow. I'll use is_array=3
                s->is_array = 3; 
                // Store struct name in name? No, I need a better way.
                // For now, I'll assume name is enough.
            }
            break;

        case AST_ARRAY_DECL:
            symtab_add(tab, node->data.array_decl.name, 0, node->data.array_decl.type, 1, node->data.array_decl.size);
            break;

        case AST_ASSIGN: {
            Symbol *s = symtab_lookup(tab, node->data.assign.name);
            if (!s) { exit(1); }
            semantic_analyze(node->data.assign.value, tab);
            break;
        }

        case AST_MEMBER_ACCESS: {
            semantic_analyze(node->data.member_access.ptr, tab);
            // We need to know which struct this is.
            // Simplified: if ptr is AST_VARIABLE, look up its struct type.
            node->eval_type = TYPE_INT;
            break;
        }

        case AST_MEMBER_ASSIGN:
            semantic_analyze(node->data.member_assign.obj, tab);
            semantic_analyze(node->data.member_assign.value, tab);
            break;

        case AST_FUNC_DECL: {
            symtab_add(tab, node->data.func_decl.name, 0, node->data.func_decl.return_type, 0, 0);
            Symbol *s = symtab_lookup(tab, node->data.func_decl.name);
            s->is_array = 2;
            SymbolTable *func_tab = symtab_new(tab);
            for (int i = 0; i < node->data.func_decl.param_count; i++) {
                symtab_add(func_tab, node->data.func_decl.params[i].name, 0, node->data.func_decl.params[i].type, 0, 0);
            }
            semantic_analyze(node->data.func_decl.body, func_tab);
            break;
        }

        case AST_EXTERN_DECL: {
            symtab_add(tab, node->data.func_decl.name, 0, node->data.func_decl.return_type, 0, 0);
            Symbol *s = symtab_lookup(tab, node->data.func_decl.name);
            s->is_array = 2;
            break;
        }

        case AST_IMPORT: break;

        case AST_NS_ACCESS: {
            char mangled[256];
            sprintf(mangled, "%s_%s", node->data.ns_access.module, node->data.ns_access.name);
            Symbol *s = symtab_lookup(tab, mangled);
            if (s) node->eval_type = s->type;
            else node->eval_type = TYPE_INT;
            break;
        }

        case AST_FUNC_CALL: {
            Symbol *s = symtab_lookup(tab, node->data.func_call.name);
            for (int i = 0; i < node->data.func_call.arg_count; i++) {
                semantic_analyze(node->data.func_call.args[i], tab);
            }
            if (s) node->eval_type = s->type;
            else node->eval_type = TYPE_INT;
            break;
        }

        case AST_SYSCALL:
            for (int i = 0; i < node->data.syscall.arg_count; i++) {
                semantic_analyze(node->data.syscall.args[i], tab);
            }
            node->eval_type = TYPE_INT;
            break;

        case AST_ADDR_OF:
            semantic_analyze(node->data.addr_of.expr, tab);
            node->eval_type = TYPE_PTR;
            break;

        case AST_DEREF:
            semantic_analyze(node->data.deref.expr, tab);
            node->eval_type = TYPE_INT;
            break;

        case AST_RETURN:
            semantic_analyze(node->data.ret.expr, tab);
            break;

        case AST_BIN_OP: {
            semantic_analyze(node->data.bin_op.left, tab);
            semantic_analyze(node->data.bin_op.right, tab);
            Type left_t = node->data.bin_op.left->eval_type;
            if (is_pointer(left_t)) node->eval_type = TYPE_PTR;
            else node->eval_type = left_t;
            break;
        }

        case AST_VARIABLE: {
            Symbol *s = symtab_lookup(tab, node->data.var_name);
            if (s) node->eval_type = s->type;
            else node->eval_type = TYPE_UNKNOWN;
            break;
        }

        case AST_ARRAY_ACCESS: {
            Symbol *s = symtab_lookup(tab, node->data.array_access.name);
            semantic_analyze(node->data.array_access.index, tab);
            if (s) node->eval_type = s->type;
            break;
        }

        case AST_PRINT:
            semantic_analyze(node->data.print_expr, tab);
            node->eval_type = node->data.print_expr->eval_type;
            break;

        case AST_IF:
            semantic_analyze(node->data.if_stmt.condition, tab);
            semantic_analyze(node->data.if_stmt.then_branch, tab);
            if (node->data.if_stmt.else_branch) semantic_analyze(node->data.if_stmt.else_branch, tab);
            break;

        case AST_WHILE:
            semantic_analyze(node->data.while_stmt.condition, tab);
            semantic_analyze(node->data.while_stmt.body, tab);
            break;

        case AST_MATCH:
            semantic_analyze(node->data.match.expr, tab);
            for (int i = 0; i < node->data.match.case_count; i++) {
                semantic_analyze(node->data.match.cases[i]->data.match_case.stmt, tab);
            }
            if (node->data.match.default_case) semantic_analyze(node->data.match.default_case, tab);
            break;

        case AST_NUMBER: node->eval_type = TYPE_INT; break;
        case AST_BOOL: node->eval_type = TYPE_BOOL; break;
        case AST_FLOAT: node->eval_type = TYPE_FLOAT; break;
        case AST_STRING: node->eval_type = TYPE_STRING; break;
        default: break;
    }
}
