#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "codegen.h"

Codegen *codegen_new(FILE *out) {
    Codegen *cg = malloc(sizeof(Codegen));
    cg->out = out;
    cg->tab = symtab_new();
    cg->stack_pos = 0;
    return cg;
}

static int label_count = 0;

static int new_label() {
    return ++label_count;
}

static void gen_expression(Codegen *cg, ASTNode *node) {
    if (node->type == AST_NUMBER) {
        fprintf(cg->out, "    mov rax, %d\n", node->data.number);
        fprintf(cg->out, "    push rax\n");
    } else if (node->type == AST_BOOL) {
        fprintf(cg->out, "    mov rax, %d\n", node->data.bool_val);
        fprintf(cg->out, "    push rax\n");
    } else if (node->type == AST_VARIABLE) {
        Symbol *s = symtab_lookup(cg->tab, node->data.var_name);
        if (!s) {
            fprintf(stderr, "Undefined variable: %s\n", node->data.var_name);
            exit(1);
        }
        fprintf(cg->out, "    mov rax, [rbp - %d]\n", s->stack_offset);
        fprintf(cg->out, "    push rax\n");
    } else if (node->type == AST_BIN_OP) {
        gen_expression(cg, node->data.bin_op.left);
        gen_expression(cg, node->data.bin_op.right);
        fprintf(cg->out, "    pop rbx\n");
        fprintf(cg->out, "    pop rax\n");
        
        char *op = node->data.bin_op.op;
        if (strcmp(op, "+") == 0) {
            fprintf(cg->out, "    add rax, rbx\n");
        } else if (strcmp(op, "==") == 0) {
            fprintf(cg->out, "    cmp rax, rbx\n");
            fprintf(cg->out, "    sete al\n");
            fprintf(cg->out, "    movzx rax, al\n");
        } else if (strcmp(op, "!=") == 0) {
            fprintf(cg->out, "    cmp rax, rbx\n");
            fprintf(cg->out, "    setne al\n");
            fprintf(cg->out, "    movzx rax, al\n");
        } else if (strcmp(op, "<") == 0) {
            fprintf(cg->out, "    cmp rax, rbx\n");
            fprintf(cg->out, "    setl al\n");
            fprintf(cg->out, "    movzx rax, al\n");
        } else if (strcmp(op, ">") == 0) {
            fprintf(cg->out, "    cmp rax, rbx\n");
            fprintf(cg->out, "    setg al\n");
            fprintf(cg->out, "    movzx rax, al\n");
        } else if (strcmp(op, "&&") == 0) {
            fprintf(cg->out, "    and rax, rbx\n");
        } else if (strcmp(op, "||") == 0) {
            fprintf(cg->out, "    or rax, rbx\n");
        }
        fprintf(cg->out, "    push rax\n");
    }
}

void codegen_generate(Codegen *cg, ASTNode *node) {
    if (node->type == AST_PROGRAM) {
        static int top_level = 1;
        int is_top = top_level;
        if (is_top) {
            top_level = 0;
            fprintf(cg->out, "section .text\n");
            fprintf(cg->out, "global _start\n\n");
            fprintf(cg->out, "_start:\n");
            fprintf(cg->out, "    push rbp\n");
            fprintf(cg->out, "    mov rbp, rsp\n");
            fprintf(cg->out, "    sub rsp, 128 ; Reserve space for vars\n\n");
        }

        for (int i = 0; i < node->data.program.count; i++) {
            codegen_generate(cg, node->data.program.nodes[i]);
        }

        if (is_top) {
            fprintf(cg->out, "\n    mov rsp, rbp\n");
            fprintf(cg->out, "    pop rbp\n");
            fprintf(cg->out, "    mov rax, 60 ; syscall exit\n");
            fprintf(cg->out, "    xor rdi, rdi\n");
            fprintf(cg->out, "    syscall\n\n");

            // Simple print helper
            fprintf(cg->out, "print_num:\n");
            fprintf(cg->out, "    push rbp\n");
            fprintf(cg->out, "    mov rbp, rsp\n");
            fprintf(cg->out, "    sub rsp, 32\n");
            fprintf(cg->out, "    mov rax, rdi\n");
            fprintf(cg->out, "    mov rcx, 10\n");
            fprintf(cg->out, "    mov rsi, rbp\n");
            fprintf(cg->out, "    dec rsi\n");
            fprintf(cg->out, "    mov byte [rsi], 10 ; newline\n");
            fprintf(cg->out, ".loop:\n");
            fprintf(cg->out, "    xor rdx, rdx\n");
            fprintf(cg->out, "    div rcx\n");
            fprintf(cg->out, "    add dl, '0'\n");
            fprintf(cg->out, "    dec rsi\n");
            fprintf(cg->out, "    mov [rsi], dl\n");
            fprintf(cg->out, "    test rax, rax\n");
            fprintf(cg->out, "    jnz .loop\n");
            fprintf(cg->out, "    mov rax, 1 ; syscall write\n");
            fprintf(cg->out, "    mov rdi, 1 ; stdout\n");
            fprintf(cg->out, "    mov rdx, rbp\n");
            fprintf(cg->out, "    sub rdx, rsi\n");
            fprintf(cg->out, "    syscall\n");
            fprintf(cg->out, "    leave\n");
            fprintf(cg->out, "    ret\n");
            top_level = 1; // Reset for next run if needed
        }
    } else if (node->type == AST_VAR_DECL) {
        gen_expression(cg, node->data.var_decl.value);
        int size = 8;
        if (node->data.var_decl.type == TYPE_INT8 || node->data.var_decl.type == TYPE_UINT8) size = 1;
        else if (node->data.var_decl.type == TYPE_INT32 || node->data.var_decl.type == TYPE_UINT32) size = 4;

        cg->stack_pos += 8;
        symtab_add(cg->tab, node->data.var_decl.name, cg->stack_pos, node->data.var_decl.type);
        fprintf(cg->out, "    pop rax\n");
        if (size == 1) fprintf(cg->out, "    mov [rbp - %d], al\n", cg->stack_pos);
        else if (size == 4) fprintf(cg->out, "    mov [rbp - %d], eax\n", cg->stack_pos);
        else fprintf(cg->out, "    mov [rbp - %d], rax\n", cg->stack_pos);
    } else if (node->type == AST_PRINT) {
        gen_expression(cg, node->data.print_expr);
        fprintf(cg->out, "    pop rdi\n");
        fprintf(cg->out, "    call print_num\n");
    } else if (node->type == AST_IF) {
        gen_expression(cg, node->data.if_stmt.condition);
        fprintf(cg->out, "    pop rax\n");
        fprintf(cg->out, "    test rax, rax\n");
        
        int else_label = new_label();
        int end_label = new_label();
        
        fprintf(cg->out, "    jz .L%d\n", node->data.if_stmt.else_branch ? else_label : end_label);
        
        codegen_generate(cg, node->data.if_stmt.then_branch);
        fprintf(cg->out, "    jmp .L%d\n", end_label);
        
        if (node->data.if_stmt.else_branch) {
            fprintf(cg->out, ".L%d:\n", else_label);
            codegen_generate(cg, node->data.if_stmt.else_branch);
        }
        
        fprintf(cg->out, ".L%d: ; end if\n", end_label);
    } else if (node->type == AST_MATCH) {
        gen_expression(cg, node->data.match.expr);
        fprintf(cg->out, "    pop rax ; match expression\n");
        
        int end_label = new_label();
        int *case_labels = malloc(sizeof(int) * node->data.match.case_count);
        
        for (int i = 0; i < node->data.match.case_count; i++) {
            case_labels[i] = new_label();
            fprintf(cg->out, "    cmp rax, %d\n", node->data.match.cases[i]->data.match_case.val);
            fprintf(cg->out, "    je .L%d\n", case_labels[i]);
        }
        
        int default_label = -1;
        if (node->data.match.default_case) {
            default_label = new_label();
            fprintf(cg->out, "    jmp .L%d\n", default_label);
        } else {
            fprintf(cg->out, "    jmp .L%d\n", end_label);
        }
        
        for (int i = 0; i < node->data.match.case_count; i++) {
            fprintf(cg->out, ".L%d:\n", case_labels[i]);
            codegen_generate(cg, node->data.match.cases[i]->data.match_case.stmt);
            fprintf(cg->out, "    jmp .L%d\n", end_label);
        }
        
        if (node->data.match.default_case) {
            fprintf(cg->out, ".L%d:\n", default_label);
            codegen_generate(cg, node->data.match.default_case);
            fprintf(cg->out, "    jmp .L%d\n", end_label);
        }
        
        fprintf(cg->out, ".L%d: ; end match\n", end_label);
        free(case_labels);
    }
}
gen_generate(cg, node->data.match.cases[i]->data.match_case.stmt);
            fprintf(cg->out, "    jmp .L%d\n", end_label);
        }
        
        if (node->data.match.default_case) {
            fprintf(cg->out, ".L%d:\n", default_label);
            codegen_generate(cg, node->data.match.default_case);
            fprintf(cg->out, "    jmp .L%d\n", end_label);
        }
        
        fprintf(cg->out, ".L%d: ; end match\n", end_label);
        free(case_labels);
    }
}
