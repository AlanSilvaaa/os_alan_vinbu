section .data
    vertical_line db "+----------------------+", 0xA
    vertical_line_len equ $ - vertical_line

    and_msg db "| And: | "
    and_len equ $ - and_msg

    or_msg db "| Or: | "
    or_len equ $ - or_msg

    xor_msg db "| Xor: | "
    xor_len equ $ - xor_msg

    not1_msg db "| Not of first input: | "
    not1_len equ $ - not1_msg

    not2_msg db "| Not of second input: | "
    not2_len equ $ - not2_msg

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
    call and_op
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

    ; Prints the string "Xor:"
    mov rax, 1
    mov rdi, 1
    mov rsi, xor_msg
    mov rdx, xor_len
    syscall

    ; Makes the XOR operation and prints the result
    call xor_op
    push rax
    call print_result
    pop rax

    ; Prints the string "Not of the first input:"
    mov rax, 1
    mov rdi, 1
    mov rsi, not1_msg
    mov rdx, not1_len
    syscall

    ; Makes the NOT operation and prints the result
    call not1_op
    push rax
    call print_result
    pop rax

    ; Prints the string "Not of the second input:"
    mov rax, 1
    mov rdi, 1
    mov rsi, not2_msg
    mov rdx, not2_len
    syscall

    ; Makes the NOT operation and prints the result
    call not2_op
    push rax
    call print_result
    pop rax

    ; Exits
    mov rax, 60
    mov rdi, 0
    syscall


; and_op
; +-------------------------------------+
; Does the AND operation: (A * B)
and_op:
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


; xor_op
; +-------------------------------------+
; Does the XOR operation: (A + B) - 2(A * B)
xor_op:
    ; copy the inputs to rax and rdi
    mov rax, [rsp+8]
    mov rdi, [rsp+16]

    ; 2(A * B)
    mov rcx, rax
    imul rcx, rdi
    imul rcx, 2

    ; (A + B)
    add rax, rdi

    ; (A + B) - 2(A * B)
    sub rax, rcx
    ret


; not1_op
; +-------------------------------------+
; Does the NOT operation: (1 - A)
not1_op:
    mov rdi, [rsp+16] ; first input
    mov rax, 1
    sub rax, rdi
    ret


; not2_op
; +-------------------------------------+
; Does the NOT operation: (1 - A)
not2_op:
    mov rdi, [rsp+8] ; second input
    mov rax, 1
    sub rax, rdi
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

    ; prints the result
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

    ; prints the result
    mov rax, 1
    mov rdi, 1
    lea rsi, vertical_line 
    mov rdx, vertical_line_len
    syscall

    pop rax ; Pops the value pushed before so the stack pointer is again the return address.
    ret
