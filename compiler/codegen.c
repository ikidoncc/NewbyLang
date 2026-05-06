#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "codegen.h"

Codegen *codegen_new(FILE *out, const char *module_name) {
    Codegen *cg = malloc(sizeof(Codegen));
    cg->out = out;
    cg->tab = symtab_new(NULL);
    cg->stack_pos = 0;
    cg->module_name = strdup(module_name);
    cg->struct_count = 0;
    cg->emit_entry = 1;
    return cg;
}

static int label_count = 0;
static int data_count = 0;

static int new_label() {
    return ++label_count;
}

static StructMeta *find_struct(Codegen *cg, const char *name) {
    if (!name) return NULL;
    for (int i = 0; i < cg->struct_count; i++) {
        if (strcmp(cg->structs[i].name, name) == 0) return &cg->structs[i];
    }
    return NULL;
}

static void gen_expression(Codegen *cg, ASTNode *node);

static void gen_lvalue(Codegen *cg, ASTNode *node) {
    if (node->type == AST_VARIABLE) {
        Symbol *s = symtab_lookup(cg->tab, node->data.var_name);
        if (!s) { fprintf(stderr, "Undefined variable: %s\n", node->data.var_name); exit(1); }
        fprintf(cg->out, "    lea rax, [rbp - %d]\n", s->stack_offset);
        fprintf(cg->out, "    push rax\n");
    } else if (node->type == AST_DEREF) {
        gen_expression(cg, node->data.deref.expr);
    } else if (node->type == AST_ARRAY_ACCESS) {
        Symbol *s = symtab_lookup(cg->tab, node->data.array_access.name);
        gen_expression(cg, node->data.array_access.index);
        fprintf(cg->out, "    pop rbx\n    imul rbx, 8\n    mov rcx, rbp\n    sub rcx, %d\n    add rcx, rbx\n    push rcx\n", s->stack_offset);
    } else if (node->type == AST_MEMBER_ACCESS) {
        gen_lvalue(cg, node->data.member_access.ptr);
        fprintf(cg->out, "    pop rax\n");
        int offset = 0;
        for(int i=0; i<cg->struct_count; i++) {
            for(int j=0; j<cg->structs[i].member_count; j++) {
                if(strcmp(cg->structs[i].members[j].name, node->data.member_access.member) == 0) {
                    offset = cg->structs[i].members[j].offset;
                    break;
                }
            }
        }
        fprintf(cg->out, "    add rax, %d\n", offset);
        fprintf(cg->out, "    push rax\n");
    }
}

static void gen_expression(Codegen *cg, ASTNode *node) {
    if (!node) return;
    if (node->type == AST_NUMBER) {
        fprintf(cg->out, "    mov rax, %d\n", node->data.number);
        fprintf(cg->out, "    push rax\n");
    } else if (node->type == AST_BOOL) {
        fprintf(cg->out, "    mov rax, %d\n", node->data.bool_val);
        fprintf(cg->out, "    push rax\n");
    } else if (node->type == AST_FLOAT) {
        int id = ++data_count;
        fprintf(cg->out, "section .data\n    f%d dq %f\nsection .text\n", id, node->data.float_val);
        fprintf(cg->out, "    mov rax, [f%d]\n", id);
        fprintf(cg->out, "    push rax\n");
    } else if (node->type == AST_STRING) {
        int id = ++data_count;
        fprintf(cg->out, "section .data\n    s%d db '%s', 0\nsection .text\n", id, node->data.string_val);
        fprintf(cg->out, "    mov rax, s%d\n", id);
        fprintf(cg->out, "    push rax\n");
    } else if (node->type == AST_VARIABLE) {
        Symbol *s = symtab_lookup(cg->tab, node->data.var_name);
        if (!s) { fprintf(stderr, "Undefined variable: %s\n", node->data.var_name); exit(1); }
        fprintf(cg->out, "    mov rax, [rbp - %d]\n", s->stack_offset);
        fprintf(cg->out, "    push rax\n");
    } else if (node->type == AST_MEMBER_ACCESS) {
        gen_lvalue(cg, node);
        fprintf(cg->out, "    pop rax\n    mov rax, [rax]\n    push rax\n");
    } else if (node->type == AST_NS_ACCESS) {
        char mangled[512];
        sprintf(mangled, "%s_%s", node->data.ns_access.module, node->data.ns_access.name);
        fprintf(cg->out, "    extern %s\n    mov rax, [%s]\n    push rax\n", mangled, mangled);
    } else if (node->type == AST_ARRAY_ACCESS) {
        Symbol *s = symtab_lookup(cg->tab, node->data.array_access.name);
        gen_expression(cg, node->data.array_access.index);
        fprintf(cg->out, "    pop rbx\n    imul rbx, 8\n    mov rcx, rbp\n    sub rcx, %d\n    mov rax, [rcx + rbx]\n    push rax\n", s->stack_offset);
    } else if (node->type == AST_FUNC_CALL) {
        char *reg_params[] = {"rdi", "rsi", "rdx", "rcx", "r8", "r9"};
        for (int i = 0; i < node->data.func_call.arg_count; i++) gen_expression(cg, node->data.func_call.args[i]);
        for (int i = node->data.func_call.arg_count - 1; i >= 0; i--) fprintf(cg->out, "    pop %s\n", reg_params[i]);
        fprintf(cg->out, "    call %s\n    push rax\n", node->data.func_call.name);
    } else if (node->type == AST_SYSCALL) {
        char *reg_syscall[] = {"rax", "rdi", "rsi", "rdx", "r10", "r8", "r9"};
        for (int i = 0; i < node->data.syscall.arg_count; i++) gen_expression(cg, node->data.syscall.args[i]);
        for (int i = node->data.syscall.arg_count - 1; i >= 0; i--) fprintf(cg->out, "    pop %s\n", reg_syscall[i]);
        fprintf(cg->out, "    syscall\n    push rax\n");
    } else if (node->type == AST_SIZEOF) {
        int size = 8;
        if (node->data.size_of.struct_name) {
            StructMeta *s = find_struct(cg, node->data.size_of.struct_name);
            if (s) size = s->size;
        }
        fprintf(cg->out, "    mov rax, %d\n", size);
        fprintf(cg->out, "    push rax\n");
    } else if (node->type == AST_ADDR_OF) {
        gen_lvalue(cg, node->data.addr_of.expr);
    } else if (node->type == AST_DEREF) {
        gen_expression(cg, node->data.deref.expr);
        fprintf(cg->out, "    pop rax\n    mov rax, [rax]\n    push rax\n");
    } else if (node->type == AST_BIN_OP) {
        gen_expression(cg, node->data.bin_op.left);
        gen_expression(cg, node->data.bin_op.right);
        fprintf(cg->out, "    pop rbx\n    pop rax\n");
        char *op = node->data.bin_op.op;
        if (strcmp(op, "+") == 0) {
            if (is_pointer(node->data.bin_op.left->eval_type)) fprintf(cg->out, "    imul rbx, 8\n");
            else if (is_pointer(node->data.bin_op.right->eval_type)) fprintf(cg->out, "    imul rax, 8\n");
            fprintf(cg->out, "    add rax, rbx\n");
        } else if (strcmp(op, "-") == 0) {
            if (is_pointer(node->data.bin_op.left->eval_type)) fprintf(cg->out, "    imul rbx, 8\n");
            fprintf(cg->out, "    sub rax, rbx\n");
        } else if (strcmp(op, "*") == 0) fprintf(cg->out, "    imul rax, rbx\n");
        else if (strcmp(op, "/") == 0) { fprintf(cg->out, "    xor rdx, rdx\n    idiv rbx\n"); }
        else if (strcmp(op, "==") == 0) { fprintf(cg->out, "    cmp rax, rbx\n    sete al\n    movzx rax, al\n"); }
        else if (strcmp(op, "<") == 0) { fprintf(cg->out, "    cmp rax, rbx\n    setl al\n    movzx rax, al\n"); }
        else if (strcmp(op, ">") == 0) { fprintf(cg->out, "    cmp rax, rbx\n    setg al\n    movzx rax, al\n"); }
        else if (strcmp(op, "&&") == 0) fprintf(cg->out, "    and rax, rbx\n");
        else if (strcmp(op, "||") == 0) fprintf(cg->out, "    or rax, rbx\n");
        fprintf(cg->out, "    push rax\n");
    }
}

void codegen_generate(Codegen *cg, ASTNode *node) {
    if (!node) return;
    if (node->type == AST_PROGRAM) {
        static int top_level = 1;
        if (top_level) {
            top_level = 0;
            fprintf(cg->out, "section .data\n    t_str db 'true', 10\n    f_str db 'false', 10\n");
            fprintf(cg->out, "section .text\nglobal _start\n\n");
            for (int i = 0; i < node->data.program.count; i++) {
                ASTNode *n = node->data.program.nodes[i];
                if (n->type == AST_EXTERN_DECL || n->type == AST_FUNC_DECL || n->type == AST_STRUCT_DEF) codegen_generate(cg, n);
            }
            if (cg->emit_entry) fprintf(cg->out, "_start:\n    push rbp\n    mov rbp, rsp\n    sub rsp, 4096\n\n");
            for (int i = 0; i < node->data.program.count; i++) {
                ASTNode *n = node->data.program.nodes[i];
                if (n->type != AST_EXTERN_DECL && n->type != AST_FUNC_DECL && n->type != AST_STRUCT_DEF) codegen_generate(cg, n);
            }
            if (cg->emit_entry) fprintf(cg->out, "\n    mov rax, 60\n    xor rdi, rdi\n    syscall\n\n");
            fprintf(cg->out, "print_num:\n    push rbp\n    mov rbp, rsp\n    sub rsp, 32\n    mov rax, rdi\n    mov rcx, 10\n    mov rsi, rbp\n    dec rsi\n    mov byte [rsi], 10\n.loop:\n    xor rdx, rdx\n    div rcx\n    add dl, '0'\n    dec rsi\n    mov [rsi], dl\n    test rax, rax\n    jnz .loop\n    mov rax, 1\n    mov rdi, 1\n    mov rdx, rbp\n    sub rdx, rsi\n    syscall\n    leave\n    ret\n\n");
            fprintf(cg->out, "print_bool:\n    push rbp\n    mov rbp, rsp\n    test rdi, rdi\n    jz .f\n    mov rax, 1\n    mov rdi, 1\n    mov rsi, t_str\n    mov rdx, 5\n    syscall\n    jmp .e\n.f:\n    mov rax, 1\n    mov rdi, 1\n    mov rsi, f_str\n    mov rdx, 6\n    syscall\n.e:\n    leave\n    ret\n\n");
            fprintf(cg->out, "print_string:\n    push rbp\n    mov rbp, rsp\n    mov rsi, rdi\n    xor rdx, rdx\n.l:\n    cmp byte [rsi + rdx], 0\n    je .d\n    inc rdx\n    jmp .l\n.d:\n    mov rax, 1\n    mov rdi, 1\n    syscall\n    mov rax, 1\n    mov rdi, 1\n    push 10\n    mov rsi, rsp\n    mov rdx, 1\n    syscall\n    add rsp, 8\n    leave\n    ret\n");
            top_level = 1;
        } else {
            SymbolTable *old_tab = cg->tab;
            cg->tab = symtab_new(old_tab);
            for (int i = 0; i < node->data.program.count; i++) codegen_generate(cg, node->data.program.nodes[i]);
            cg->tab = old_tab;
        }
    } else if (node->type == AST_STRUCT_DEF) {
        StructMeta *s = &cg->structs[cg->struct_count++];
        s->name = strdup(node->data.struct_def.name);
        s->member_count = node->data.struct_def.member_count;
        int current_offset = 0;
        for (int i = 0; i < s->member_count; i++) {
            s->members[i].name = strdup(node->data.struct_def.members[i].name);
            s->members[i].type = node->data.struct_def.members[i].type;
            s->members[i].offset = current_offset;
            current_offset += 8;
        }
        s->size = current_offset;
    } else if (node->type == AST_FUNC_DECL) {
        int over_label = new_label();
        fprintf(cg->out, "    jmp L%d\n", over_label);
        if (node->data.func_decl.is_pub) fprintf(cg->out, "%s_%s:\n", cg->module_name, node->data.func_decl.name);
        else fprintf(cg->out, "%s:\n", node->data.func_decl.name);
        fprintf(cg->out, "    push rbp\n    mov rbp, rsp\n    sub rsp, 1024\n");
        SymbolTable *old_tab = cg->tab;
        cg->tab = symtab_new(old_tab);
        int old_stack = cg->stack_pos; cg->stack_pos = 0;
        char *reg_params[] = {"rdi", "rsi", "rdx", "rcx", "r8", "r9"};
        for (int i = 0; i < node->data.func_decl.param_count; i++) {
            cg->stack_pos += 8;
            symtab_add(cg->tab, node->data.func_decl.params[i].name, cg->stack_pos, node->data.func_decl.params[i].type, 0, 0);
            fprintf(cg->out, "    mov [rbp - %d], %s\n", cg->stack_pos, reg_params[i]);
        }
        codegen_generate(cg, node->data.func_decl.body);
        fprintf(cg->out, "    leave\n    ret\nL%d:\n", over_label);
        cg->tab = old_tab; cg->stack_pos = old_stack;
    } else if (node->type == AST_IMPORT) {
    } else if (node->type == AST_EXTERN_DECL) {
        fprintf(cg->out, "extern %s\n", node->data.func_decl.name);
    } else if (node->type == AST_RETURN) {
        gen_expression(cg, node->data.ret.expr);
        fprintf(cg->out, "    pop rax\n    leave\n    ret\n");
    } else if (node->type == AST_VAR_DECL) {
        int size = 8;
        if (node->data.var_decl.struct_name) {
            StructMeta *s = find_struct(cg, node->data.var_decl.struct_name);
            if (s) size = s->size;
        }
        if (node->data.var_decl.value) {
            gen_expression(cg, node->data.var_decl.value);
            fprintf(cg->out, "    pop rax\n");
        }
        cg->stack_pos += size;
        symtab_add(cg->tab, node->data.var_decl.name, cg->stack_pos, node->data.var_decl.type, 0, 0);
        if (node->data.var_decl.value) fprintf(cg->out, "    mov [rbp - %d], rax\n", cg->stack_pos);
    } else if (node->type == AST_ARRAY_DECL) {
        cg->stack_pos += node->data.array_decl.size * 8;
        symtab_add(cg->tab, node->data.array_decl.name, cg->stack_pos, node->data.array_decl.type, 1, node->data.array_decl.size);
    } else if (node->type == AST_ASSIGN) {
        gen_expression(cg, node->data.assign.value);
        Symbol *s = symtab_lookup(cg->tab, node->data.assign.name);
        fprintf(cg->out, "    pop rax\n    mov [rbp - %d], rax\n", s->stack_offset);
    } else if (node->type == AST_MEMBER_ASSIGN) {
        gen_expression(cg, node->data.member_assign.value);
        gen_lvalue(cg, node->data.member_assign.obj);
        fprintf(cg->out, "    pop rbx\n    pop rax\n");
        int offset = 0;
        for(int i=0; i<cg->struct_count; i++) {
            for(int j=0; j<cg->structs[i].member_count; j++) {
                if(strcmp(cg->structs[i].members[j].name, node->data.member_assign.member) == 0) {
                    offset = cg->structs[i].members[j].offset;
                    break;
                }
            }
        }
        fprintf(cg->out, "    mov [rbx + %d], rax\n", offset);
    } else if (node->type == AST_ARRAY_ASSIGN) {
        gen_expression(cg, node->data.array_assign.value);
        gen_expression(cg, node->data.array_assign.index);
        fprintf(cg->out, "    pop rbx\n    pop rax\n");
        Symbol *s = symtab_lookup(cg->tab, node->data.array_assign.name);
        fprintf(cg->out, "    imul rbx, 8\n    mov rcx, rbp\n    sub rcx, %d\n    mov [rcx + rbx], rax\n", s->stack_offset);
    } else if (node->type == AST_DEREF_ASSIGN) {
        gen_expression(cg, node->data.deref_assign.value);
        gen_expression(cg, node->data.deref_assign.ptr);
        fprintf(cg->out, "    pop rbx\n    pop rax\n    mov [rbx], rax\n");
    } else if (node->type == AST_PRINT) {
        gen_expression(cg, node->data.print_expr);
        fprintf(cg->out, "    pop rdi\n");
        if (node->eval_type == TYPE_BOOL) fprintf(cg->out, "    call print_bool\n");
        else if (node->eval_type == TYPE_STRING) fprintf(cg->out, "    call print_string\n");
        else fprintf(cg->out, "    call print_num\n");
    } else if (node->type == AST_IF) {
        gen_expression(cg, node->data.if_stmt.condition);
        fprintf(cg->out, "    pop rax\n    test rax, rax\n");
        int l1 = new_label(), l2 = new_label();
        fprintf(cg->out, "    jz L%d\n", node->data.if_stmt.else_branch ? l1 : l2);
        codegen_generate(cg, node->data.if_stmt.then_branch);
        fprintf(cg->out, "    jmp L%d\n", l2);
        if (node->data.if_stmt.else_branch) { fprintf(cg->out, "L%d:\n", l1); codegen_generate(cg, node->data.if_stmt.else_branch); }
        fprintf(cg->out, "L%d:\n", l2);
    } else if (node->type == AST_WHILE) {
        int l1 = new_label(), l2 = new_label();
        fprintf(cg->out, "L%d:\n", l1);
        gen_expression(cg, node->data.while_stmt.condition);
        fprintf(cg->out, "    pop rax\n    test rax, rax\n    jz L%d\n", l2);
        codegen_generate(cg, node->data.while_stmt.body);
        fprintf(cg->out, "    jmp L%d\nL%d:\n", l1, l2);
    } else if (node->type == AST_FUNC_CALL) {
        gen_expression(cg, node);
        fprintf(cg->out, "    pop rax\n");
    } else if (node->type == AST_SYSCALL) {
        gen_expression(cg, node);
        fprintf(cg->out, "    pop rax\n");
    }
}
