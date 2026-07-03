;
;  Copyright (C) 2026 HJimmyK(Jericho Knox)
;
;  This file is part of LAMMP.
;
;  LAMMP is free software: you can redistribute it and/or modify it under
;  the terms of the GNU Lesser General Public License (LGPL) as published
;   by the Free Software Foundation; either version 3 of the License, or
;  (at your option) any later version.
;
;  This program is distributed WITHOUT ANY WARRANTY. 
;
;  See <https://www.gnu.org/licenses/>.

;
; mp_limb_t lmmp_shlnot_(mp_ptr dst, mp_srcptr numa, mp_size_t na, mp_size_t shl);

bits 64
default rel

%macro win64_args 0
%ifdef LAMMP_ASM_WIN
    xchg rcx,r9
%endif
%endmacro

%macro lin64_args 0
%ifndef LAMMP_ASM_WIN
    mov r8,rdx
    mov r9,rdi
    mov rdx,rsi
%endif
%endmacro

%macro prolog 0
 push r12
 push r13
%endmacro

%macro epilog 0
 pop r13
 pop r12
 ret
%endmacro

section .text
align 16

global lmmp_shlnot_
lmmp_shlnot_:
    xor rax,rax
    lin64_args
    win64_args
    and cl,63
    prolog

    mov r10,[rdx+r8*8-8]
    shld rax,r10,cl

    test r8b,1
    jnz .lab_lx1

.lab_lx0:
    test r8b,2
    jnz .lab_l10

.lab_l00:
    lea rdx,[rdx+r8*8-32]
    lea r9,[r9+r8*8]
    mov r12,r10
    mov r13,[rdx+16]
    jmp .lab_ll0

.lab_l10:
    lea rdx,[rdx+r8*8-16]
    lea r9,[r9+r8*8-16]
    mov r11,[rdx]
    jmp .lab_ll2

.lab_lx1:
    test r8b,2
    jnz .lab_l11

.lab_l01:
    dec r8
    jz .lab_e01
    lea rdx,[rdx+r8*8-32]
    lea r9,[r9+r8*8]
    mov r12,[rdx+24]
    mov r13,[rdx+16]
    shld r10,r12,cl
    not r10
    mov [r9],r10
    jmp .lab_ll1

.lab_e01:
    shl r10,cl
    not r10
    mov [r9],r10
    epilog

.lab_l11:
    mov r13,r10
    lea rdx,[rdx+r8*8-24]
    lea r9,[r9+r8*8+8]
    mov r10,[rdx+8]
    mov r11,[rdx]
    shld r13,r10,cl
    not r13
    jmp .lab_ll3

align 16
.lab_top:
    mov r12,[rdx-8]
    mov r13,[rdx-16]
    lea rdx,[rdx-32]
    shld r10,r11,cl
    shld r11,r12,cl
    not r10
    not r11
    mov [r9+8],r10
    mov [r9],r11

.lab_ll1:
.lab_ll0:
    mov r10,[rdx+8]
    mov r11,[rdx]
    shld r12,r13,cl
    shld r13,r10,cl
    not r12
    not r13
    mov [r9-8],r12

.lab_ll3:
    mov [r9-16],r13
    lea r9,[r9-32]

.lab_ll2:
    sub r8,4
    ja .lab_top

.lab_end:
    shld r10,r11,cl
    shl r11,cl
    not r10
    not r11
    mov [r9+8],r10
    mov [r9],r11
    epilog