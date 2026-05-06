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

typedef struct EnumDef {
    char *name;
    char *variants[16];
    int variant_count;
} EnumDef;

static StructDef global_structs[16];
static int global_struct_count = 0;

static EnumDef global_enums[16];
static int global_enum_count = 0;

static StructDef *find_struct_def(const char *name) {
    if (!name) return NULL;
    for (int i = 0; i < global_struct_count; i++) {
        if (strcmp(global_structs[i].name, name) == 0) return &global_structs[i];
    }
    return NULL;
}

static EnumDef *find_enum_def(const char *name) {
    if (!name) return NULL;
    for (int i = 0; i < global_enum_count; i++) {
        if (strcmp(global_enums[i].name, name) == 0) return &global_enums[i];
    }
    return NULL;
}

static char *current_struct = NULL;

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
            char *old_struct = current_struct;
            current_struct = sd->name;
            for (int i = 0; i < node->data.struct_def.method_count; i++) {
                semantic_analyze(node->data.struct_def.methods[i], tab);
            }
            current_struct = old_struct;
            break;
        }

        case AST_ENUM_DEF: {
            EnumDef *ed = &global_enums[global_enum_count++];
            ed->name = strdup(node->data.enum_def.name);
            ed->variant_count = node->data.enum_def.variant_count;
            for (int i = 0; i < ed->variant_count; i++) {
                ed->variants[i] = strdup(node->data.enum_def.variants[i]);
            }
            break;
        }

        case AST_VAR_DECL:
            if (node->data.var_decl.value) semantic_analyze(node->data.var_decl.value, tab);
            symtab_add(tab, node->data.var_decl.name, 0, node->data.var_decl.type, 0, 0, node->data.var_decl.struct_name);
            break;

        case AST_ARRAY_DECL:
            symtab_add(tab, node->data.array_decl.name, 0, node->data.array_decl.type, 1, node->data.array_decl.size, NULL);
            break;

        case AST_ASSIGN: {
            semantic_analyze(node->data.assign.value, tab);
            break;
        }

        case AST_MEMBER_ACCESS: {
            semantic_analyze(node->data.member_access.ptr, tab);
            // Check if it's an enum access: Enum.Variant
            if (node->data.member_access.ptr->type == AST_VARIABLE) {
                EnumDef *ed = find_enum_def(node->data.member_access.ptr->data.var_name);
                if (ed) {
                    for (int i = 0; i < ed->variant_count; i++) {
                        if (strcmp(ed->variants[i], node->data.member_access.member) == 0) {
                            // Resolve to integer constant
                            node->type = AST_NUMBER;
                            node->data.number = i;
                            node->eval_type = TYPE_INT;
                            return;
                        }
                    }
                }
            }
            node->eval_type = TYPE_INT;
            break;
        }

        case AST_MEMBER_ASSIGN:
            semantic_analyze(node->data.member_assign.obj, tab);
            semantic_analyze(node->data.member_assign.value, tab);
            break;

        case AST_FUNC_DECL: {
            if (node->data.func_decl.parent_struct) {
                char mangled[512];
                sprintf(mangled, "%s_%s", node->data.func_decl.parent_struct, node->data.func_decl.name);
                node->data.func_decl.name = strdup(mangled);
            }
            symtab_add(tab, node->data.func_decl.name, 0, node->data.func_decl.return_type, 2, 0, NULL);
            SymbolTable *func_tab = symtab_new(tab);
            char *old_struct = current_struct;
            if (node->data.func_decl.parent_struct) {
                current_struct = node->data.func_decl.parent_struct;
                symtab_add(func_tab, "self", 0, TYPE_PTR, 0, 0, current_struct);
            }
            for (int i = 0; i < node->data.func_decl.param_count; i++) {
                symtab_add(func_tab, node->data.func_decl.params[i].name, 0, node->data.func_decl.params[i].type, 0, 0, NULL);
            }
            semantic_analyze(node->data.func_decl.body, func_tab);
            current_struct = old_struct;
            break;
        }

        case AST_EXTERN_DECL: {
            symtab_add(tab, node->data.func_decl.name, 0, node->data.func_decl.return_type, 2, 0, NULL);
            break;
        }

        case AST_SELF:
            node->eval_type = TYPE_PTR;
            break;

        case AST_FUNC_CALL: {
            if (node->data.func_call.obj) {
                semantic_analyze(node->data.func_call.obj, tab);
                char *s_name = NULL;
                if (node->data.func_call.obj->type == AST_VARIABLE) {
                    Symbol *s = symtab_lookup(tab, node->data.func_call.obj->data.var_name);
                    if (s) s_name = s->struct_name;
                } else if (node->data.func_call.obj->type == AST_SELF) {
                    s_name = current_struct;
                }
                if (s_name) {
                    char mangled[512];
                    sprintf(mangled, "%s_%s", s_name, node->data.func_call.name);
                    node->data.func_call.name = strdup(mangled);
                }
            }
            for (int i = 0; i < node->data.func_call.arg_count; i++) {
                if (node->data.func_call.args[i]) semantic_analyze(node->data.func_call.args[i], tab);
            }
            Symbol *s = symtab_lookup(tab, node->data.func_call.name);
            if (s) node->eval_type = s->type;
            else node->eval_type = TYPE_INT;
            break;
        }

        case AST_SYSCALL:
            for (int i = 0; i < node->data.syscall.arg_count; i++) semantic_analyze(node->data.syscall.args[i], tab);
            node->eval_type = TYPE_INT;
            break;

        case AST_SIZEOF: node->eval_type = TYPE_INT; break;
        case AST_ADDR_OF: semantic_analyze(node->data.addr_of.expr, tab); node->eval_type = TYPE_PTR; break;
        case AST_DEREF: semantic_analyze(node->data.deref.expr, tab); node->eval_type = TYPE_INT; break;
        case AST_RETURN: semantic_analyze(node->data.ret.expr, tab); break;

        case AST_BIN_OP: {
            semantic_analyze(node->data.bin_op.left, tab);
            semantic_analyze(node->data.bin_op.right, tab);
            if (node->data.bin_op.left) node->eval_type = node->data.bin_op.left->eval_type;
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
            node->eval_type = node->data.print_expr ? node->data.print_expr->eval_type : TYPE_INT;
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
                ASTNode *c = node->data.match.cases[i];
                semantic_analyze(c->data.match_case.val_node, tab);
                // If val_node is a constant (number or enum variant resolved to number),
                // copy its value to c->data.match_case.val
                if (c->data.match_case.val_node->type == AST_NUMBER) {
                    c->data.match_case.val = c->data.match_case.val_node->data.number;
                }
                semantic_analyze(c->data.match_case.stmt, tab);
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
