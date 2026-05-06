#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "semantic.h"

typedef struct StructDef {
    char *name;
    struct { char *name; Type type; } members[16];
    int member_count;
} StructDef;

typedef struct EnumDef {
    char *name;
    struct { char *name; Type type; char *struct_name; } variants[16];
    int variant_count;
    char *generic_params[4];
    int generic_count;
    ASTNode *template_node;
} EnumDef;

static StructDef global_structs[16];
static int global_struct_count = 0;

static EnumDef global_enums[16];
static int global_enum_count = 0;

static char *global_modules[32];
static int global_module_count = 0;

void semantic_add_module(const char *name) {
    global_modules[global_module_count++] = strdup(name);
}

static int is_module(const char *name) {
    if (!name) return 0;
    for (int i = 0; i < global_module_count; i++) {
        if (strcmp(global_modules[i], name) == 0) return 1;
    }
    return 0;
}

static EnumDef *find_enum_def(const char *name) {
    if (!name) return NULL;
    for (int i = 0; i < global_enum_count; i++) {
        if (strcmp(global_enums[i].name, name) == 0) return &global_enums[i];
    }
    return NULL;
}

static char *current_struct = NULL;

int semantic_is_enum(const char *name) {
    return find_enum_def(name) != NULL;
}

void semantic_cleanup() {
    for (int i = 0; i < global_struct_count; i++) {
        free(global_structs[i].name);
        for (int j = 0; j < global_structs[i].member_count; j++) {
            free(global_structs[i].members[j].name);
        }
    }
    for (int i = 0; i < global_enum_count; i++) {
        free(global_enums[i].name);
        for (int j = 0; j < global_enums[i].variant_count; j++) {
            free(global_enums[i].variants[j].name);
            if (global_enums[i].variants[j].struct_name) free(global_enums[i].variants[j].struct_name);
        }
    }
    for (int i = 0; i < global_module_count; i++) {
        free(global_modules[i]);
    }
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
            ed->generic_count = node->data.enum_def.generic_count;
            for (int i = 0; i < ed->variant_count; i++) {
                ed->variants[i].name = strdup(node->data.enum_def.variants[i].name);
                ed->variants[i].type = node->data.enum_def.variants[i].type;
                ed->variants[i].struct_name = node->data.enum_def.variants[i].struct_name ? strdup(node->data.enum_def.variants[i].struct_name) : NULL;
            }
            for (int i = 0; i < ed->generic_count; i++) {
                ed->generic_params[i] = strdup(node->data.enum_def.generic_params[i]);
            }
            ed->template_node = node;
            break;
        }

        case AST_VAR_DECL:
            if (node->data.var_decl.value) semantic_analyze(node->data.var_decl.value, tab);
            symtab_add(tab, node->data.var_decl.name, 0, node->data.var_decl.type, 0, 0, node->data.var_decl.struct_name);
            if (node->data.var_decl.struct_name) {
                if (semantic_is_enum(node->data.var_decl.struct_name)) {
                    Symbol *s = symtab_lookup(tab, node->data.var_decl.name);
                    s->is_array = 4;
                } else {
                    Symbol *s = symtab_lookup(tab, node->data.var_decl.name);
                    s->is_array = 3;
                }
            }
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
            if (node->data.member_access.ptr->type == AST_VARIABLE) {
                EnumDef *ed = find_enum_def(node->data.member_access.ptr->data.var_name);
                if (ed) {
                    for (int i = 0; i < ed->variant_count; i++) {
                        if (strcmp(ed->variants[i].name, node->data.member_access.member) == 0) {
                            // Don't change type to AST_NUMBER here to avoid corruption,
                            // but mark eval_type and store info if needed.
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
            symtab_add(tab, node->data.func_decl.name, 0, node->data.func_decl.return_type, 2, 0, node->data.func_decl.return_struct_name);
            SymbolTable *func_tab = symtab_new(tab);
            char *old_struct = current_struct;
            if (node->data.func_decl.parent_struct) {
                current_struct = node->data.func_decl.parent_struct;
                symtab_add(func_tab, "self", 0, TYPE_PTR, 0, 0, current_struct);
            }
            for (int i = 0; i < node->data.func_decl.param_count; i++) {
                int is_enum = 0;
                if (node->data.func_decl.params[i].struct_name) {
                    if (semantic_is_enum(node->data.func_decl.params[i].struct_name)) is_enum = 4;
                }
                symtab_add(func_tab, node->data.func_decl.params[i].name, 0, node->data.func_decl.params[i].type, is_enum, 0, node->data.func_decl.params[i].struct_name);
            }
            semantic_analyze(node->data.func_decl.body, func_tab);
            current_struct = old_struct;
            break;
        }

        case AST_EXTERN_DECL: {
            symtab_add(tab, node->data.func_decl.name, 0, node->data.func_decl.return_type, 2, 0, node->data.func_decl.return_struct_name);
            break;
        }

        case AST_TRY: {
            semantic_analyze(node->data.try.expr, tab);
            node->eval_type = TYPE_INT; // Default
            char *enum_name = NULL;
            if (node->data.try.expr->type == AST_FUNC_CALL) {
                Symbol *s = symtab_lookup(tab, node->data.try.expr->data.func_call.name);
                if (s) enum_name = s->struct_name;
            } else if (node->data.try.expr->type == AST_VARIABLE) {
                Symbol *s = symtab_lookup(tab, node->data.try.expr->data.var_name);
                if (s) enum_name = s->struct_name;
            }
            if (enum_name) {
                EnumDef *ed = find_enum_def(enum_name);
                if (ed && ed->variant_count > 0) {
                    node->eval_type = ed->variants[0].type;
                }
            }
            break;
        }

        case AST_SELF:
            node->eval_type = TYPE_PTR;
            break;

        case AST_FUNC_CALL: {
            if (node->data.func_call.obj) {
                char *s_name = NULL;
                if (node->data.func_call.obj->type == AST_VARIABLE) {
                    char *name = node->data.func_call.obj->data.var_name;
                    EnumDef *ed = find_enum_def(name);
                    if (ed) {
                        for (int i = 0; i < ed->variant_count; i++) {
                            if (strcmp(ed->variants[i].name, node->data.func_call.name) == 0) {
                                node->type = AST_METHOD_CALL;
                                node->data.number = i;
                                if (node->data.func_call.arg_count > 0) {
                                    semantic_analyze(node->data.func_call.args[0], tab);
                                }
                                node->eval_type = TYPE_INT;
                                return;
                            }
                        }
                    }

                    if (is_module(name)) {
                        char mangled[512];
                        sprintf(mangled, "%s_%s", name, node->data.func_call.name);
                        free(node->data.func_call.name);
                        node->data.func_call.name = strdup(mangled);
                        node->data.func_call.obj = NULL;
                    } else {
                        semantic_analyze(node->data.func_call.obj, tab);
                        Symbol *s = symtab_lookup(tab, name);
                        if (s) s_name = s->struct_name;
                    }
                } else if (node->data.func_call.obj->type == AST_SELF) {
                    semantic_analyze(node->data.func_call.obj, tab);
                    s_name = current_struct;
                }
                
                if (s_name && node->data.func_call.obj) {
                    char mangled[512];
                    sprintf(mangled, "%s_%s", s_name, node->data.func_call.name);
                    free(node->data.func_call.name);
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
                ASTNode *vn = c->data.match_case.val_node;
                Type var_type = TYPE_INT;
                char *var_struct = NULL;
                
                if (vn->type == AST_NUMBER) {
                    c->data.match_case.val = vn->data.number;
                } else if (vn->type == AST_VARIABLE) {
                    // Simple variable match? No, must be Enum variant.
                    // But if it's just 'case Val:', it might be a variable.
                    // Simplified: for now only handle member access for enums.
                } else if (vn->type == AST_MEMBER_ACCESS) {
                    ASTNode *ptr = vn->data.member_access.ptr;
                    char *member = vn->data.member_access.member;
                    if (ptr->type == AST_VARIABLE) {
                        EnumDef *ed = find_enum_def(ptr->data.var_name);
                        if (ed) {
                            for (int k = 0; k < ed->variant_count; k++) {
                                if (strcmp(ed->variants[k].name, member) == 0) {
                                    c->data.match_case.val = k;
                                    var_type = ed->variants[k].type;
                                    var_struct = ed->variants[k].struct_name;
                                    break;
                                }
                            }
                        }
                    }
                }
                
                SymbolTable *case_tab = symtab_new(tab);
                if (c->data.match_case.capture_var) {
                    symtab_add(case_tab, c->data.match_case.capture_var, 0, var_type, 0, 0, var_struct);
                }
                semantic_analyze(c->data.match_case.stmt, case_tab);
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
