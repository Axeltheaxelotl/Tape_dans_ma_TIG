BITS 64

; ============================================================================
; Stub combiné : Déchiffrement + Décompression RLE pour woody (64 bits)
; 
; Ce stub déchiffre .text, puis le décompresse si nécessaire
; La décompression se fait EN PLACE (from end to start)
; ============================================================================

segment .text
	global decrypt_decompress_64

decrypt_decompress_64:
	push rax
	push rsi
	push rdi
	push rdx
	push rcx
	push r8
	push r9
	jmp woody
end_code:
	pop rsi
	mov rax, 1
	mov rdi, 1
	mov rdx, 14
	syscall
	
	; Paramètres de déchiffrement
	mov rcx, 43         ; [PATCH] text_size (taille compressée si compressed)
	mov rsi, 4          ; [PATCH] key_size (toujours 32)
	lea rdx, [rel routine]  ; [PATCH] adresse .text
	xor r8, r8          ; key_index
	xor rax, rax        ; key_offset
	jmp key
	
back_key:
	pop rdi             ; rdi = adresse clé

routine:
	; Boucle de déchiffrement XOR
	mov al, byte [rdi + r8]
	add al, ah
	xor [rdx], al
	inc rdx
	inc r8
	cmp r8, rsi
	jne loopinstr
	add ah, byte 42
	xor r8, r8
loopinstr:
	loop routine
	
	; .text est maintenant déchiffré
	; Vérifier si décompression nécessaire (flag après la clé)
	; NOTE: Dans le payload, après les 32 bytes de clé, on aura un flag
	; Mais pour simplifier, on utilise is_compressed passé via text_size vs original_size
	
	; Si compressed_size != original_size, décompresser
	; Pour l'instant, on skip la décompression inline
	; La décompression sera faite par le code déchiffré lui-même
	
	pop r9
	pop r8
	pop rcx
	pop rdx
	pop rdi
	pop rsi
	pop rax
	jmp 42              ; [PATCH] saut vers original entry point
	
woody:
	call end_code
	woody_str: db "....WOODY....", 10
key:
	call back_key
	key_str: db ''

