section .data
    and_msg db "And:", 0xA
    and_len equ $ - and_msg

section .text
    global _start

; section .bss
;     and_value resb 8

_start:
    ; Prints the string "And:"
    mov rax, 1
    mov rdi, 1
    mov rsi, and_msg
    mov rdx, and_len
    syscall

    ; Pushes the value 1 to the stack
    mov rax, 1
    inc rax
    push rax

    mov rax, 1
    mov rdi, 1
    mov rsi, rsp
    mov rdx, 1
    syscall

    mov rax, 60
    mov rdi, [rsp]
    syscall
