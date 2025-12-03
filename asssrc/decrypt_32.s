BITS 32

segment .text
	global decrypt_32

decrypt_32:
	push eax
	push esi
	push edi
	push edx
	push ecx
	push ebx
	jmp woody
end_code:
	pop esi
	mov eax, 4          ; sys_write (32-bit)
	mov ebx, 1          ; stdout
	mov edx, 14         ; length
	int 0x80            ; syscall 32-bit
	mov ecx, 43         ; text_size
	mov esi, 4          ; key_size
	call get_eip
get_eip:
	pop edx
	add edx, (routine - get_eip)  ; text address
	xor ebx, ebx        ; key_index
	xor eax, eax        ; key_offset
	jmp key
back_key:
	pop edi             ; key
routine:
	mov al, byte [edi + ebx]
	add al, ah
	xor [edx], al
	inc edx
	inc ebx
	cmp ebx, esi
	jne loopinstr
	add ah, byte 42
	xor ebx, ebx
loopinstr:
	loop routine
	pop ebx
	pop ecx
	pop edx
	pop edi
	pop esi
	pop eax
	jmp 42
woody:
	call end_code
	woody_str: db "....WOODY....", 10
key:
	call back_key
	key_str: db ''
