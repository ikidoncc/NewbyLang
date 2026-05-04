section .text
global _start

_start:
    push rbp
    mov rbp, rsp
    sub rsp, 128 ; Reserve space for vars

