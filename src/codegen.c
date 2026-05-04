#include <stdio.h>
#include <stdlib.h>
#include "codegen.h"

Codegen *codegen_new(FILE *out) {
    Codegen *cg = malloc(sizeof(Codegen));
    cg->out = out;
    cg->tab = symtab_new();
    cg->stack_pos = 0;
    return cg;
}

static void gen_expression(Codegen *cg, ASTNode *node) {
    if (node->type == AST_NUMBER) {
        fprintf(cg->out, "    mov rax, %d\n", node->data.number);
        fprintf(cg->out, "    push rax\n");
    } else if (node->type == AST_VARIABLE) {
        int offset = symtab_lookup(cg->tab, node->data.var_name);
        if (offset == -1) {
            fprintf(stderr, "Undefined variable: %s\n", node->data.var_name);
            exit(1);
        }
        fprintf(cg->out, "    mov rax, [rbp - %d]\n", offset);
        fprintf(cg->out, "    push rax\n");
    } else if (node->type == AST_BIN_OP) {
        gen_expression(cg, node->data.bin_op.left);
        gen_expression(cg, node->data.bin_op.right);
        fprintf(cg->out, "    pop rbx\n");
        fprintf(cg->out, "    pop rax\n");
        if (node->data.bin_op.op[0] == '+') {
            fprintf(cg->out, "    add rax, rbx\n");
        }
        fprintf(cg->out, "    push rax\n");
    }
}

void codegen_generate(Codegen *cg, ASTNode *node) {
    if (node->type == AST_PROGRAM) {
        fprintf(cg->out, "section .text\n");
        fprintf(cg->out, "global _start\n\n");
        fprintf(cg->out, "_start:\n");
        fprintf(cg->out, "    push rbp\n");
        fprintf(cg->out, "    mov rbp, rsp\n");
        fprintf(cg->out, "    sub rsp, 128 ; Reserve space for vars\n\n");

        for (int i = 0; i < node->data.program.count; i++) {
            codegen_generate(cg, node->data.program.nodes[i]);
        }

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

    } else if (node->type == AST_VAR_DECL) {
        gen_expression(cg, node->data.var_decl.value);
        cg->stack_pos += 8;
        symtab_add(cg->tab, node->data.var_decl.name, cg->stack_pos);
        fprintf(cg->out, "    pop rax\n");
        fprintf(cg->out, "    mov [rbp - %d], rax\n", cg->stack_pos);
    } else if (node->type == AST_PRINT) {
        gen_expression(cg, node->data.print_expr);
        fprintf(cg->out, "    pop rdi\n");
        fprintf(cg->out, "    call print_num\n");
    }
}
