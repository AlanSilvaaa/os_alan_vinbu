section .data
    and_msg db "And: "
    and_len equ $ - and_msg

    or_msg db "Or: "
    or_len equ $ - or_msg

    newline db 0xA

section .text
    global _start

_start:
    ; Pushes the input values (for now hardcoded to 1 and 0)
    mov rax, 1
    mov rdi, 0
    push rax
    push rdi

    ; Prints the string "And:"
    mov rax, 1
    mov rdi, 1
    mov rsi, and_msg
    mov rdx, and_len
    syscall

    ; Makes the AND operation and prints the result
    call add_op
    push rax
    call print_result
    pop rax ; Pops the result of the AND operation so in the stack there's only the two inputs.

    ; Prints the string "Or:"
    mov rax, 1
    mov rdi, 1
    mov rsi, or_msg
    mov rdx, or_len
    syscall

    ; Makes the OR operation and prints the result
    call or_op
    push rax
    call print_result
    pop rax

    ; Exits
    mov rax, 60
    mov rdi, 0
    syscall


; add_op
; +-------------------------------------+
; Does the AND operation: (A * B)
add_op:
    ; copy the inputs to rax and rdi
    mov rax, [rsp+8]
    mov rdi, [rsp+16]
    imul rax, rdi
    ; push rax ; <- This is dangerous, because it will push the result on top of stack, and when the function returns, it will jump to the address of the value we just pushed, instead of the return address.
    ret


; or_op
; +-------------------------------------+
; Does the OR operation: (A + B) - (A * B)
or_op:
    ; copy the inputs to rax and rdi
    mov rax, [rsp+8]
    mov rdi, [rsp+16]

    ; (A * B)
    mov rcx, rax
    imul rcx, rdi

    ; (A + B)
    add rax, rdi

    ; (A + B) - (A * B)
    sub rax, rcx
    ret


; print_result
; +-------------------------------------+
; Converts the result to ASCII and prints it.
print_result:
    ; Converts the result to ASCII adding + 48 (the number 0 in ASCII)
    mov rax, [rsp+8] ; When assembly calls a function, it stores a return address into the stack, so in order to read the actual value we want, we need to add 8 bytes to the stack pointer.
    add rax, 48
    push rax

    ; at this point the stack looks like this:
    ; (ASCII value, return address, operation_result, input2, input1)

    ; Prints the result
    mov rax, 1
    mov rdi, 1
    lea rsi, [rsp] 
    mov rdx, 1
    syscall

    ; prints a newline
    mov rax, 1
    mov rdi, 1
    lea rsi, newline 
    mov rdx, 1
    syscall

    pop rax ; Pops the value pushed before so the stack pointer is again the return address.
    ret
