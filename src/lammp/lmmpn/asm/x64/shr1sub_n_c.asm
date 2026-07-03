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
; mp_limb_t lmmp_shr1sub_n_(mp_ptr dst, mp_srcptr numa, mp_srcptr numb, mp_size_t n);
;
; mp_limb_t lmmp_shr1sub_nc_(mp_ptr dst, mp_srcptr numa, mp_srcptr numb, mp_size_t n, mp_limb_t c);


bits 64
default rel

%ifdef LAMMP_ASM_WIN
 %macro WIN_PROLOG 0
    push rsi
    push rdi
    push rbx
    mov rsi, rdx        ; numa
    mov rdi, rcx        ; dst
    mov rdx, r8         ; numb
    mov rcx, r9         ; n
 %endmacro

 %macro WIN_EPILOG 0
    pop rbx
    pop rdi
    pop rsi
    ret
 %endmacro

 %macro WIN_LOAD_C 0
    mov r8, [rsp + 64]
 %endmacro
%else
 %macro WIN_PROLOG 0
    push rbx
    mov rdi, rdi
    mov rsi, rsi
    mov rdx, rdx
    mov rcx, rcx
 %endmacro

 %macro WIN_EPILOG 0
    pop rbx
    ret
 %endmacro

 %macro WIN_LOAD_C 0
    mov r8, r8
 %endmacro
%endif

%define ADCSBB sbb

section .text
align 16

; ---------------------------------------------------------
; 带借位减法右移 lmmp_shr1sub_nc_
; ---------------------------------------------------------
global lmmp_shr1sub_nc_
lmmp_shr1sub_nc_:
    WIN_PROLOG
    WIN_LOAD_C
    xor rax, rax
    neg r8
    mov rbx, [rsi]
    sbb rbx, [rdx]
    jmp entry

align 16

; ---------------------------------------------------------
; 无借位减法右移 lmmp_shr1sub_n_
; ---------------------------------------------------------
global lmmp_shr1sub_n_
lmmp_shr1sub_n_:
    WIN_PROLOG
    xor rax, rax
    mov rbx, [rsi]
    sub rbx, [rdx]

entry:
    rcr rbx, 1
    adc rax, rax
    mov r11, rcx
    and r11, 3
    cmp r11, 1
    je do_quad

n1:
    cmp r11, 2
    jne n2
    add rbx, rbx
    mov r10, [rsi+8]
    sbb r10, [rdx+8]
    lea rsi, [rsi+8]
    lea rdx, [rdx+8]
    lea rdi, [rdi+8]
    rcr r10, 1
    rcr rbx, 1
    mov [rdi-8], rbx
    jmp cj1

n2:
    cmp r11, 3
    jne n3
    add rbx, rbx
    mov r9, [rsi+8]
    mov r10, [rsi+16]
    sbb r9, [rdx+8]
    sbb r10, [rdx+16]
    lea rsi, [rsi+16]
    lea rdx, [rdx+16]
    lea rdi, [rdi+16]
    rcr r10, 1
    rcr r9, 1
    rcr rbx, 1
    mov [rdi-16], rbx
    jmp cj2

n3:
    dec rcx
    add rbx, rbx
    mov r8, [rsi+8]
    mov r9, [rsi+16]
    sbb r8, [rdx+8]
    sbb r9, [rdx+16]
    mov r10, [rsi+24]
    sbb r10, [rdx+24]
    lea rsi, [rsi+24]
    lea rdx, [rdx+24]
    lea rdi, [rdi+24]
    rcr r10, 1
    rcr r9, 1
    rcr r8, 1
    rcr rbx, 1
    mov [rdi-24], rbx
    mov [rdi-16], r8

cj2:
    mov [rdi-8], r9
cj1:
    mov rbx, r10

do_quad:
    shr rcx, 2
    je end_loop

align 16
loop_top:
    add rbx, rbx
    mov r8, [rsi+8]
    mov r9, [rsi+16]
    sbb r8, [rdx+8]
    sbb r9, [rdx+16]
    mov r10, [rsi+24]
    mov r11, [rsi+32]
    sbb r10, [rdx+24]
    sbb r11, [rdx+32]
    lea rsi, [rsi+32]
    lea rdx, [rdx+32]
    rcr r11, 1
    rcr r10, 1
    rcr r9, 1
    rcr r8, 1
    rcr rbx, 1
    mov [rdi],    rbx
    mov [rdi+8],  r8
    mov [rdi+16], r9
    mov [rdi+24], r10
    mov rbx, r11
    lea rdi, [rdi+32]
    dec rcx
    jne loop_top

end_loop:
    mov [rdi], rbx
    WIN_EPILOG