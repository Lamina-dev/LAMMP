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
    mov rax, rcx    
    mul rdx        
    mov [r8], rax   
    mov [r8+8], rdx 
    ret          

	ALIGN 16
lmmp_mullh_ endp

	ALIGN 16
lmmp_mulmod_ulong_ proc
    push rbx
    mov rbx, r8       
    mov rax, rcx     
    mul rdx           
    xor rcx, rcx     
    div rbx           
    mov [r9], rax     
    mov rax, rdx       
    pop rbx
    ret
lmmp_mulmod_ulong_ endp

end
