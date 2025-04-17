section .data
    and_msg db "And:", 0xA
    and_len equ $ - and_msg

section .text
    global _start

_start:
    ; Prints the string "And:"
    mov rax, 1
    mov rdi, 1
    mov rsi, and_msg
    mov rdx, and_len
    syscall

    ; Pushes the input values (for now hardcoded to 1 and 0)
    mov rax, 1
    mov rdi, 0
    push rax
    push rdi

    ; Multiplies the two values on the stack (AND operation)
    mov rax, [rsp]
    mov rdi, [rsp+8]
    imul rax, rdi
    push rax

    ; Converts the result to ASCII using +48 (the number 0 in ASCII)
    mov rax, [rsp]; 
    add rax, 48
    push rax

    call print_result

    ; Exits
    mov rax, 60
    mov rdi, 0
    syscall

print_result:
    ; Prints the result
    mov rax, 1
    mov rdi, 1
    lea rsi, [rsp+8] ; When assembly calls a function, it stores a return address into the stack, so in order to read the actual value we want, we need to add 8 bytes to the stack pointer.
    mov rdx, 1
    syscall
    ret

