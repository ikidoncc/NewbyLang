#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "semantic.h"

void semantic_analyze(ASTNode *node, SymbolTable *tab) {
    if (!node) return;

    switch (node->type) {
        case AST_PROGRAM:
            for (int i = 0; i < node->data.program.count; i++) {
                semantic_analyze(node->data.program.nodes[i], tab);
            }
            break;

        case AST_VAR_DECL:
            semantic_analyze(node->data.var_decl.value, tab);
            if (node->data.var_decl.value->eval_type != node->data.var_decl.type) {
                fprintf(stderr, "Semantic Error: Type mismatch in declaration of '%s'. Expected %s, got %s\n",
                        node->data.var_decl.name, type_to_string(node->data.var_decl.type),
                        type_to_string(node->data.var_decl.value->eval_type));
                exit(1);
            }
            // Add to symtab (mocking stack offset for now, codegen will handle actual offset)
            symtab_add(tab, node->data.var_decl.name, 0, node->data.var_decl.type);
            break;

        case AST_BIN_OP:
            semantic_analyze(node->data.bin_op.left, tab);
            semantic_analyze(node->data.bin_op.right, tab);
            
            Type left_t = node->data.bin_op.left->eval_type;
            Type right_t = node->data.bin_op.right->eval_type;
            char *op = node->data.bin_op.op;

            if (strcmp(op, "+") == 0) {
                if (left_t != TYPE_INT || right_t != TYPE_INT) {
                    fprintf(stderr, "Semantic Error: '+' operator only supports int, got %s and %s\n",
                            type_to_string(left_t), type_to_string(right_t));
                    exit(1);
                }
                node->eval_type = TYPE_INT;
            } else if (strcmp(op, "==") == 0 || strcmp(op, "!=") == 0 ||
                       strcmp(op, "<") == 0 || strcmp(op, ">") == 0) {
                if (left_t != TYPE_INT || right_t != TYPE_INT) {
                    fprintf(stderr, "Semantic Error: '%s' operator only supports int, got %s and %s\n",
                            op, type_to_string(left_t), type_to_string(right_t));
                    exit(1);
                }
                node->eval_type = TYPE_BOOL;
            } else if (strcmp(op, "&&") == 0 || strcmp(op, "||") == 0) {
                if (left_t != TYPE_BOOL || right_t != TYPE_BOOL) {
                    fprintf(stderr, "Semantic Error: '%s' operator only supports bool, got %s and %s\n",
                            op, type_to_string(left_t), type_to_string(right_t));
                    exit(1);
                }
                node->eval_type = TYPE_BOOL;
            }
            break;

        case AST_VARIABLE: {
            Symbol *s = symtab_lookup(tab, node->data.var_name);
            if (!s) {
                fprintf(stderr, "Semantic Error: Undefined variable '%s'\n", node->data.var_name);
                exit(1);
            }
            node->eval_type = s->type;
            break;
        }

        case AST_PRINT:
            semantic_analyze(node->data.print_expr, tab);
            break;

        case AST_MATCH:
            semantic_analyze(node->data.match.expr, tab);
            if (node->data.match.expr->eval_type != TYPE_INT) {
                fprintf(stderr, "Semantic Error: 'match' expression must be of type int\n");
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

        default:
            break;
    }
}
