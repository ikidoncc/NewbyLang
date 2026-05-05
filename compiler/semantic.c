#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "semantic.h"

static int is_integer_type(Type t) {
    return t == TYPE_INT || t == TYPE_INT8 || t == TYPE_INT32 || 
           t == TYPE_UINT || t == TYPE_UINT8 || t == TYPE_UINT32;
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

        case AST_VAR_DECL:
            semantic_analyze(node->data.var_decl.value, tab);
            if (node->data.var_decl.value->eval_type != node->data.var_decl.type) {
                fprintf(stderr, "Semantic Error [%d:%d]: Type mismatch in declaration of '%s'. Expected %s, got %s\n",
                        node->line, node->col, node->data.var_decl.name, 
                        type_to_string(node->data.var_decl.type),
                        type_to_string(node->data.var_decl.value->eval_type));
                exit(1);
            }
            symtab_add(tab, node->data.var_decl.name, 0, node->data.var_decl.type, 0, 0);
            break;

        case AST_ARRAY_DECL:
            symtab_add(tab, node->data.array_decl.name, 0, node->data.array_decl.type, 1, node->data.array_decl.size);
            break;

        case AST_ASSIGN: {
            Symbol *s = symtab_lookup(tab, node->data.assign.name);
            if (!s) {
                fprintf(stderr, "Semantic Error [%d:%d]: Assignment to undefined variable '%s'\n",
                        node->line, node->col, node->data.assign.name);
                exit(1);
            }
            if (s->is_array) {
                fprintf(stderr, "Semantic Error [%d:%d]: Cannot assign to array '%s' without index\n",
                        node->line, node->col, node->data.assign.name);
                exit(1);
            }
            semantic_analyze(node->data.assign.value, tab);
            if (node->data.assign.value->eval_type != s->type) {
                fprintf(stderr, "Semantic Error [%d:%d]: Type mismatch in assignment to '%s'. Expected %s, got %s\n",
                        node->line, node->col, node->data.assign.name,
                        type_to_string(s->type),
                        type_to_string(node->data.assign.value->eval_type));
                exit(1);
            }
            break;
        }

        case AST_ARRAY_ASSIGN: {
            Symbol *s = symtab_lookup(tab, node->data.array_assign.name);
            if (!s || !s->is_array) {
                fprintf(stderr, "Semantic Error [%d:%d]: '%s' is not an array or undefined\n",
                        node->line, node->col, node->data.array_assign.name);
                exit(1);
            }
            semantic_analyze(node->data.array_assign.index, tab);
            if (!is_integer_type(node->data.array_assign.index->eval_type)) {
                fprintf(stderr, "Semantic Error [%d:%d]: Array index must be an integer\n",
                        node->line, node->col);
                exit(1);
            }
            semantic_analyze(node->data.array_assign.value, tab);
            if (node->data.array_assign.value->eval_type != s->type) {
                fprintf(stderr, "Semantic Error [%d:%d]: Type mismatch in array assignment. Expected %s, got %s\n",
                        node->line, node->col, type_to_string(s->type),
                        type_to_string(node->data.array_assign.value->eval_type));
                exit(1);
            }
            break;
        }

        case AST_FUNC_DECL: {
            // Add function to current scope (global)
            symtab_add(tab, node->data.func_decl.name, 0, node->data.func_decl.return_type, 0, 0);
            Symbol *s = symtab_lookup(tab, node->data.func_decl.name);
            s->is_array = 2; // Hack: 2 means function
            
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
            s->is_array = 2; // Function
            break;
        }

        case AST_FUNC_CALL: {
            Symbol *s = symtab_lookup(tab, node->data.func_call.name);
            if (!s || s->is_array != 2) {
                fprintf(stderr, "Semantic Error [%d:%d]: Undefined function '%s'\n",
                        node->line, node->col, node->data.func_call.name);
                exit(1);
            }
            for (int i = 0; i < node->data.func_call.arg_count; i++) {
                semantic_analyze(node->data.func_call.args[i], tab);
            }
            node->eval_type = s->type;
            break;
        }

        case AST_SYSCALL: {
            for (int i = 0; i < node->data.syscall.arg_count; i++) {
                semantic_analyze(node->data.syscall.args[i], tab);
            }
            node->eval_type = TYPE_INT;
            break;
        }

        case AST_RETURN:
            semantic_analyze(node->data.ret.expr, tab);
            node->eval_type = node->data.ret.expr->eval_type;
            break;

        case AST_BIN_OP:
            semantic_analyze(node->data.bin_op.left, tab);
            semantic_analyze(node->data.bin_op.right, tab);
            
            Type left_t = node->data.bin_op.left->eval_type;
            Type right_t = node->data.bin_op.right->eval_type;
            char *op = node->data.bin_op.op;

            if (strcmp(op, "+") == 0 || strcmp(op, "-") == 0 || strcmp(op, "*") == 0 || strcmp(op, "/") == 0) {
                if (is_integer_type(left_t) && is_integer_type(right_t)) {
                    node->eval_type = left_t;
                } else if (left_t == TYPE_FLOAT && right_t == TYPE_FLOAT) {
                    node->eval_type = TYPE_FLOAT;
                } else {
                    fprintf(stderr, "Semantic Error [%d:%d]: '%s' operator incompatible types: %s and %s\n",
                            node->line, node->col, op, type_to_string(left_t), type_to_string(right_t));
                    exit(1);
                }
            } else if (strcmp(op, "==") == 0 || strcmp(op, "!=") == 0 ||
                       strcmp(op, "<") == 0 || strcmp(op, ">") == 0) {
                if ((is_integer_type(left_t) && is_integer_type(right_t)) ||
                    (left_t == TYPE_FLOAT && right_t == TYPE_FLOAT)) {
                    node->eval_type = TYPE_BOOL;
                } else {
                    fprintf(stderr, "Semantic Error [%d:%d]: '%s' operator incompatible types: %s and %s\n",
                            node->line, node->col, op, type_to_string(left_t), type_to_string(right_t));
                    exit(1);
                }
            } else if (strcmp(op, "&&") == 0 || strcmp(op, "||") == 0) {
                if (left_t != TYPE_BOOL || right_t != TYPE_BOOL) {
                    fprintf(stderr, "Semantic Error [%d:%d]: '%s' operator only supports bool, got %s and %s\n",
                            node->line, node->col, op, type_to_string(left_t), type_to_string(right_t));
                    exit(1);
                }
                node->eval_type = TYPE_BOOL;
            }
            break;

        case AST_VARIABLE: {
            Symbol *s = symtab_lookup(tab, node->data.var_name);
            if (!s) {
                fprintf(stderr, "Semantic Error [%d:%d]: Undefined variable '%s'\n", 
                        node->line, node->col, node->data.var_name);
                exit(1);
            }
            node->eval_type = s->type;
            break;
        }

        case AST_ARRAY_ACCESS: {
            Symbol *s = symtab_lookup(tab, node->data.array_access.name);
            if (!s || !s->is_array) {
                fprintf(stderr, "Semantic Error [%d:%d]: '%s' is not an array or undefined\n",
                        node->line, node->col, node->data.array_access.name);
                exit(1);
            }
            semantic_analyze(node->data.array_access.index, tab);
            if (!is_integer_type(node->data.array_access.index->eval_type)) {
                fprintf(stderr, "Semantic Error [%d:%d]: Array index must be an integer\n",
                        node->line, node->col);
                exit(1);
            }
            node->eval_type = s->type;
            break;
        }

        case AST_PRINT:
            semantic_analyze(node->data.print_expr, tab);
            node->eval_type = node->data.print_expr->eval_type;
            break;

        case AST_IF:
            semantic_analyze(node->data.if_stmt.condition, tab);
            if (node->data.if_stmt.condition->eval_type != TYPE_BOOL) {
                fprintf(stderr, "Semantic Error [%d:%d]: 'if' condition must be of type bool, got %s\n",
                        node->line, node->col, type_to_string(node->data.if_stmt.condition->eval_type));
                exit(1);
            }
            semantic_analyze(node->data.if_stmt.then_branch, tab);
            if (node->data.if_stmt.else_branch) {
                semantic_analyze(node->data.if_stmt.else_branch, tab);
            }
            break;

        case AST_WHILE:
            semantic_analyze(node->data.while_stmt.condition, tab);
            if (node->data.while_stmt.condition->eval_type != TYPE_BOOL) {
                fprintf(stderr, "Semantic Error [%d:%d]: 'while' condition must be of type bool, got %s\n",
                        node->line, node->col, type_to_string(node->data.while_stmt.condition->eval_type));
                exit(1);
            }
            semantic_analyze(node->data.while_stmt.body, tab);
            break;

        case AST_MATCH:
            semantic_analyze(node->data.match.expr, tab);
            if (node->data.match.expr->eval_type != TYPE_INT) {
                fprintf(stderr, "Semantic Error [%d:%d]: 'match' expression must be of type int\n",
                        node->line, node->col);
                exit(1);
            }
            for (int i = 0; i < node->data.match.case_count; i++) {
                semantic_analyze(node->data.match.cases[i]->data.match_case.stmt, tab);
            }
            if (node->data.match.default_case) {
                semantic_analyze(node->data.match.default_case, tab);
            }
            break;

        case AST_NUMBER:
            node->eval_type = TYPE_INT;
            break;

        case AST_BOOL:
            node->eval_type = TYPE_BOOL;
            break;

        case AST_FLOAT:
            node->eval_type = TYPE_FLOAT;
            break;

        case AST_STRING:
            node->eval_type = TYPE_STRING;
            break;

        default:
            break;
    }
}
