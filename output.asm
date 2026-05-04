section .text
global _start

_start:
    push rbp
    mov rbp, rsp
    sub rsp, 128 ; Reserve space for vars

    mov rax, 3
    push rax
    pop rax
    mov [rbp - 8], rax
    mov rax, [rbp - 8]
    push rax
    mov rax, 5
    push rax
    pop rbx
    pop rax
    add rax, rbx
    push rax
    pop rax
    mov [rbp - 16], rax
    mov rax, [rbp - 16]
    push rax
    pop rdi
    call print_num
    mov rax, [rbp - 8]
    push rax
    pop rdi
    call print_num

    mov rsp, rbp
    pop rbp
    mov rax, 60 ; syscall exit
    xor rdi, rdi
    syscall

print_num:
    push rbp
    mov rbp, rsp
    sub rsp, 32
    mov rax, rdi
    mov rcx, 10
    mov rsi, rbp
    dec rsi
    mov byte [rsi], 10 ; newline
.loop:
    xor rdx, rdx
    div rcx
    add dl, '0'
    dec rsi
    mov [rsi], dl
    test rax, rax
    jnz .loop
    mov rax, 1 ; syscall write
    mov rdi, 1 ; stdout
    mov rdx, rbp
    sub rdx, rsi
    syscall
    leave
    ret
