; LAMMP - Copyright (C) 2025-2026 HJimmyK(Jericho Knox)
; This file is part of lammp, under the GNU LGPL v2 license.
; See LICENSE in the project root for the full license text.
;
; lmmp_addmul_1_ : [numa, n] += [numb, n] * b

bits 64
default rel

section .text
global lmmp_addmul_1_

lmmp_addmul_1_:
%ifdef LAMMP_ASM_WIN
    push    rsi
    push    rdi
    push    rbx
    ; Windows: rcx=numa, rdx=numb, r8=n, r9=b
    mov     rdi, rcx
    mov     rsi, rdx
    mov     rcx, r9                 ; b -> rcx
    ; r8 = n
%else
    push    rbx
    ; Linux: rdi=numa, rsi=numb, rdx=n, rcx=b
    mov     r8, rdx                 ; n -> r8
    ; rdi, rsi, rcx already set
%endif

    mov     rax, [rsi]              ; load first limb (unused later, just for initial mul)
    mov     ebx, 1

    lea     rdi, [rdi + r8*8]       ; rdi = numa + n
    lea     rsi, [rsi + r8*8]       ; rsi = numb + n

    test    r8b, 1
    jnz     .odd_n

.even_n:
    xor     r11d, r11d              ; carry = 0
    test    r8b, 2
    jnz     .even_tail2

.even_tail0:
    sub     rbx, r8                 ; rbx = 1 - n
    mul     rcx
    mov     r9, rdx
    mov     r8, [rdi + rbx*8 - 8]
    jmp     .loop_entry0

.even_tail2:
    neg     rbx
    sub     rbx, r8                 ; rbx = -1 - n
    mul     rcx
    mov     r8, [rdi + rbx*8 + 8]
    mov     r9, rdx
    jmp     .loop_entry2

.odd_n:
    xor     r9d, r9d
    test    r8b, 2
    jnz     .odd_tail3

.odd_tail1:
    add     rbx, rbx
    sub     rbx, r8
    jnc     .single_limb
    mul     rcx
    mov     r10, [rdi + rbx*8 - 16]
    mov     r11, rdx
    jmp     .loop_entry1

.odd_tail3:
    xor     ebx, ebx
    sub     rbx, r8
    mul     rcx
    mov     r10, [rdi + rbx*8]
    jmp     .loop_entry3

    align   16
; ----- Main loop (4 limbs per iteration) -----
.loop_top:
.loop_entry1:
    add     r10, rax
    mov     rax, [rsi + rbx*8 - 8]
    adc     r11, 0
    mul     rcx
    add     r10, r9
    mov     r9, rdx
    mov     r8, [rdi + rbx*8 - 8]
    adc     r11, 0
    mov     [rdi + rbx*8 - 16], r10

.loop_entry0:
    add     r8, rax
    adc     r9, 0
    mov     rax, [rsi + rbx*8]
    mul     rcx
    mov     r10, [rdi + rbx*8]
    add     r8, r11
    mov     [rdi + rbx*8 - 8], r8
    adc     r9, 0

.loop_entry3:
    mov     r11, rdx
    add     r10, rax
    mov     rax, [rsi + rbx*8 + 8]
    adc     r11, 0
    mul     rcx
    mov     r8, [rdi + rbx*8 + 8]
    add     r10, r9
    mov     r9, rdx
    mov     [rdi + rbx*8], r10
    adc     r11, 0

.loop_entry2:
    add     r8, rax
    adc     r9, 0
    mov     rax, [rsi + rbx*8 + 16]
    mul     rcx
    mov     r10, [rdi + rbx*8 + 16]
    add     r8, r11
    mov     r11, rdx
    adc     r9, 0
    mov     [rdi + rbx*8 + 8], r8

    add     rbx, 4
    jnc     .loop_top

    ; epilogue
    add     r10, rax
    adc     r11, 0
    add     r10, r9
    adc     r11, 0
    mov     [rdi - 8], r10
    mov     rax, r11

    pop     rbx
%ifdef LAMMP_ASM_WIN
    pop     rdi
    pop     rsi
%endif
    ret

    align   16
.single_limb:
    mul     rcx
    add     [rdi - 8], rax
    mov     rax, rdx
    adc     rax, 0

    pop     rbx
%ifdef LAMMP_ASM_WIN
    pop     rdi
    pop     rsi
%endif
    ret