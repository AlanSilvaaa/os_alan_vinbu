section .data
    box_corner db "+"
    box_corner_len equ 1

    box_line db "-"
    box_line_len equ 1

    box_outline db "|"
    box_outline_len equ 1

    whitespace db " "
    whitespace_len equ 1

    title db "| Truth table "
    title_len equ $ - title

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
    width_box_line dq 30

section .bss
    fill_box_count resq 1
    current_len resq 1

section .text
    global _start


_start:
    ; Pushes the input values (for now hardcoded to 1 and 0)
    mov rax, 1
    mov rdi, 0
    push rax
    push rdi

    ; prints top of the box (+------------+)
    call print_horizontal_line

    ; Prints the string "Truth table"
    mov rax, 1
    mov rdi, 1
    mov rsi, title
    mov rdx, title_len
    syscall

    ; Prints a whitespace
    mov rax, 1
    mov rdi, 1
    mov rsi, whitespace
    mov rdx, 1
    syscall

    ; Saves the length of the string "Truth table" to the current_len variable
    mov rax, title_len
    mov [current_len], rax

    call fill_box

    ; Prints a newline
    mov rax, 1
    mov rdi, 1
    mov rsi, newline
    mov rdx, 1
    syscall

    ; prints top of the box (+------------+)
    call print_horizontal_line

    ; Prints the string "And:"
    mov rax, 1
    mov rdi, 1
    mov rsi, and_msg
    mov rdx, and_len
    syscall

    ; Saves the length of the string "And:" to the current_len variable
    mov rax, and_len
    mov [current_len], rax

    ; Makes the AND operation and prints the result
    call and_op
    push rax
    call print_result
    pop rax ; Pops the result of the AND operation so in the stack there's only the two inputs.


    ; prints top of the box (+------------+)
    call print_horizontal_line
    
    ; Prints the string "Or:"
    mov rax, 1
    mov rdi, 1
    mov rsi, or_msg
    mov rdx, or_len
    syscall

    ; Saves the length of the string "Or:" to the current_len variable
    mov rax, or_len
    mov [current_len], rax

    ; Makes the OR operation and prints the result
    call or_op
    push rax
    call print_result
    pop rax

    ; prints top of the box (+------------+)
    call print_horizontal_line

    ; Prints the string "Xor:"
    mov rax, 1
    mov rdi, 1
    mov rsi, xor_msg
    mov rdx, xor_len
    syscall

    ; Saves the length of the string "Xor:" to the current_len variable
    mov rax, xor_len
    mov [current_len], rax

    ; Makes the XOR operation and prints the result
    call xor_op
    push rax
    call print_result
    pop rax

    ; prints top of the box (+------------+)
    call print_horizontal_line

    ; Prints the string "Not of the first input:"
    mov rax, 1
    mov rdi, 1
    mov rsi, not1_msg
    mov rdx, not1_len
    syscall

    ; Saves the length of the string "Not of first input:" to the current_len variable
    mov rax, not1_len
    mov [current_len], rax

    ; Makes the NOT operation and prints the result
    call not1_op
    push rax
    call print_result
    pop rax

    ; prints top of the box (+------------+)
    call print_horizontal_line

    ; Prints the string "Not of the second input:"
    mov rax, 1
    mov rdi, 1
    mov rsi, not2_msg
    mov rdx, not2_len
    syscall

    ; Saves the length of the string "Not of second input" to the current_len variable
    mov rax, not2_len
    mov [current_len], rax

    ; Makes the NOT operation and prints the result
    call not2_op
    push rax
    call print_result
    pop rax

    ; prints top of the box (+------------+)
    call print_horizontal_line

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

    ; fills the space with whitespaces and closes the box
    call fill_box

    ; prints a newline
    mov rax, 1
    mov rdi, 1
    lea rsi, newline 
    mov rdx, 1
    syscall

    pop rax ; Pops the value pushed before so the stack pointer is again the return address.
    ret


; print_horizontal_line
; +-------------------------------------+
; Prints the horizontal line of the box made of the box corner (+) and the box line (-).
; The width of the line is set by the width_box_line variable.
print_horizontal_line:
    ; prints corner of the box (+)
    mov rax, 1
    mov rdi, 1
    mov rsi, box_corner
    mov rdx, box_corner_len
    syscall

    ; prints the line of the box (-------)
    call print_table_line

    ; closes the corner of the box (+)
    mov rax, 1
    mov rdi, 1
    mov rsi, box_corner
    mov rdx, box_corner_len
    syscall

    ; prints newline
    mov rax, 1
    mov rdi, 1
    mov rsi, newline
    mov rdx, 1
    syscall
    ret


; print_table_line
; +-------------------------------------+
; Prints the characters of the box line (-) until the width of the line is reached.
print_table_line:
    mov rax, 1
    mov rdi, 1
    mov rsi, box_line
    mov rdx, box_line_len
    syscall

    dec qword [width_box_line]
    mov rax, [width_box_line]
    cmp rax, 0
    jg print_table_line

    mov qword [width_box_line], 30 ; resets the width of the box line to his original value
    ret

 
; fill_box
; +-------------------------------------+
; Fills the box with whitespaces until the width of the line is reached.
; It calculates the number of whitespaces to print by subtracting the 
; space used by the label of the operation (current_len) from the width of the line.
fill_box:
    ; substracts the length of the string from the width of the line
    mov rax, [width_box_line]
    sub rax, [current_len]
    mov [fill_box_count], rax

    call whitespace_loop

    ; prints the outline of the box (|)
    mov rax, 1
    mov rdi, 1
    mov rsi, box_outline 
    mov rdx, 1
    syscall
    ret


; whitespace_loop
; +-------------------------------------+
; Prints the whitespaces until the fill_box_count variable is 0.
whitespace_loop:
    mov rax, 1
    mov rdi, 1
    mov rsi, whitespace
    mov rdx, whitespace_len 
    syscall

    dec qword [fill_box_count]
    mov rax, [fill_box_count]
    cmp rax, 0
    jg whitespace_loop
    ret
