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
; void lmmp_not_(mp_ptr dst, mp_srcptr numa, mp_size_t na);

bits 64
default rel


%ifdef LAMMP_ASM_WIN
 %macro PROLOG 0
    push rsi
    push rdi
    push rbx
    mov rdi, rcx        ; dst
    mov rsi, rdx        ; numa
    mov rdx, r8         ; na
 %endmacro
 %macro EPILOG 0
    pop rbx
    pop rdi
    pop rsi
    ret
 %endmacro
%else
 %macro PROLOG 0
    push rbx
 %endmacro
 %macro EPILOG 0
    pop rbx
    ret
 %endmacro
%endif

section .text
align 16

global lmmp_not_
lmmp_not_:
    PROLOG

    mov r10, [rsi]          ; 读第一个 limb
    mov rax, rdx            ; na
    mov r8, rdx             ; 备份 na
    lea rcx, [rdi + rdx*8]  ; dst 末尾
    lea rdx, [rsi + rdx*8]  ; numa 末尾
    neg r8                  ; 负索引

    and al, 3               ; 余数 0/1/2/3
    je .lab_b00
    cmp al, 2
    jc .lab_b01
    je .lab_b10

.lab_b11:
    not r10
    mov [rcx + r8*8], r10
    dec r8
    jmp .lab_e11

.lab_b10:
    add r8, -2
    jmp .lab_e10

.lab_b01:
    not r10
    mov [rcx + r8*8], r10
    inc r8
    jz .lab_ret

align 16
.lab_oop:
    mov r10, [rdx + r8*8]
.lab_b00:
    mov r11, [rdx + r8*8 + 8]
    not r10
    not r11
    mov [rcx + r8*8], r10
    mov [rcx + r8*8 + 8], r11
.lab_e11:
    mov r10, [rdx + r8*8 + 16]
.lab_e10:
    mov r11, [rdx + r8*8 + 24]
    not r10
    not r11
    mov [rcx + r8*8 + 16], r10
    mov [rcx + r8*8 + 24], r11

    add r8, 4
    jnc .lab_oop

.lab_ret:
    EPILOG