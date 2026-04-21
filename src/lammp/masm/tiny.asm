 ; LAMMP - Copyright (C) 2025-2026 HJimmyK(Jericho Knox)
 ; This file is part of lammp, under the GNU LGPL v2 license.
 ; See LICENSE in the project root for the full license text.
 
include <asm_windows>

.code
	ALIGN 16
lmmp_limb_bits_ proc
	mov rax,-1
	bsr rx0,rx0
	cmovnz rax,rx0
	inc rax
	ret
lmmp_limb_bits_ endp

	ALIGN 16
lmmp_leading_zeros_ proc
	mov rax,-1
	bsr rx0,rx0
	cmovnz rax,rx0
	not rax
	add rax,64
	ret
lmmp_leading_zeros_ endp

	ALIGN 16
lmmp_tailing_zeros_ proc
	mov rax,64
	bsf rx0,rx0
	cmovnz rax,rx0
	ret
lmmp_tailing_zeros_ endp
	
	ALIGN 16
lmmp_limb_popcnt_ proc
    xor rax, rax
    popcnt rax, rx0
    ret
lmmp_limb_popcnt_ endp

	ALIGN 16
lmmp_mulh_ proc
	mov rax,rx1
	mul rx0
	mov rax,rdx
	ret
lmmp_mulh_ endp

	ALIGN 16
lmmp_mullh_ proc
    mov rax, rx0    
    mul rx1        
    mov [rx2], rax   
    mov [rx2+8], rdx
    ret          
lmmp_mullh_ endp

	ALIGN 16
lmmp_mulmod_ulong_ proc
    mov rax, rx0     
    mul rx1   
    div rx2    
    mov [rx3], rax     
    mov rax, rdx    
    ret
lmmp_mulmod_ulong_ endp

end
