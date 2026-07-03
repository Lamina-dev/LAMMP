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


bits 64
default rel

%ifdef LAMMP_ASM_WIN
  %define win
  %define lin ;
  %define rx0 rcx
  %define rx1 rdx
  %define rx2 r8
  %define rx3 r9
%else
  %define win ;
  %define lin
  %define rx0 rdi
  %define rx1 rsi
  %define rx2 rdx
  %define rx3 rcx
%endif

section .text

;----------------------------------------------------------------------
; mp_size_t lmmp_limb_bits_ (mp_limb_t x)
; 返回 x 的二进制位数 (0 返回 0)
;----------------------------------------------------------------------
ALIGN 16
global lmmp_limb_bits_
lmmp_limb_bits_:
    mov    rax, -1
    bsr    rx0, rx0
    cmovnz rax, rx0
    inc    rax
    ret

;----------------------------------------------------------------------
; mp_size_t lmmp_leading_zeros_ (mp_limb_t x)
; 返回 x 的前导零个数 (x=0 返回 64)
;----------------------------------------------------------------------
ALIGN 16
global lmmp_leading_zeros_
lmmp_leading_zeros_:
    mov    rax, -1
    bsr    rx0, rx0
    cmovnz rax, rx0
    not    rax
    add    rax, 64
    ret

;----------------------------------------------------------------------
; mp_size_t lmmp_tailing_zeros_ (mp_limb_t x)
; 返回 x 的尾部零个数 (x=0 返回 64)
;----------------------------------------------------------------------
ALIGN 16
global lmmp_tailing_zeros_
lmmp_tailing_zeros_:
    mov    rax, 64
    bsf    rx0, rx0
    cmovnz rax, rx0
    ret

;----------------------------------------------------------------------
; mp_size_t lmmp_limb_popcnt_ (mp_limb_t x)
; 返回 x 中置 1 的位数
;----------------------------------------------------------------------
ALIGN 16
global lmmp_limb_popcnt_
lmmp_limb_popcnt_:
    xor    rax, rax
    popcnt rax, rx0
    ret

;----------------------------------------------------------------------
; mp_limb_t lmmp_mulh_ (mp_limb_t a, mp_limb_t b)
; 返回 a * b 的高 64 位
;----------------------------------------------------------------------
ALIGN 16
global lmmp_mulh_
lmmp_mulh_:
    mov    rax, rx1
    mul    rx0
    mov    rax, rdx
    ret

;----------------------------------------------------------------------
; void lmmp_mullh_ (mp_limb_t a, mp_limb_t b, mp_ptr dst)
; 计算 a * b，将低 64 位存入 dst[0]，高 64 位存入 dst[1]
;----------------------------------------------------------------------
ALIGN 16
global lmmp_mullh_
lmmp_mullh_:
    mov    rax, rx0
    mul    rx1
    mov    [rx2], rax
    mov    [rx2+8], rdx
    ret

;----------------------------------------------------------------------
; ulong lmmp_mulmod_ulong_(ulong a, ulong b, ulong mod, ulongp q);
; 计算 (a * b) / d，商存入 *q，返回余数
; 注意：调用者必须保证商不超过 64 位，否则会触发除法异常
;----------------------------------------------------------------------
ALIGN 16
global lmmp_mulmod_ulong_
lmmp_mulmod_ulong_:
    mov    rax, rx0
    mul    rx1
    div    rx2
    mov    [rx3], rax
    mov    rax, rdx
    ret