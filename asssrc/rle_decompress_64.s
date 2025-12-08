BITS 64

global rle_decompress_stub_64

; Fonction: rle_decompress_stub_64
; Arguments (System V AMD64 ABI):
;   rdi = compressed data (pointeur)
;   rsi = compressed size
;   rdx = output buffer (pointeur)
;   rcx = output max size
; Retour:
;   rax = taille décompressée, ou 0 si erreur

rle_decompress_stub_64:
    push rbp
    mov rbp, rsp
    push rbx
    push r12
    push r13

    ; Sauvegarder les arguments
    mov r12, rdi            ; r12 = compressed data
    mov r13, rcx            ; r13 = output max size

    xor r8, r8              ; r8 = in_pos
    xor r9, r9              ; r9 = out_pos

.loop:
    cmp r8, rsi             ; if (in_pos >= compressed_size)
    jge .done

    ; Lire le byte
    movzx rax, byte [r12 + r8]
    inc r8

    test al, 0x80           ; Tester bit 7
    jz .literal

.rle_mode:
    ; Mode RLE: [count|0x80][value]
    and rax, 0x7F           ; count = byte & 0x7F
    mov rbx, rax            ; rbx = count

    cmp r8, rsi             ; Vérifier qu'on peut lire value
    jge .error

    movzx r10, byte [r12 + r8]  ; r10 = value
    inc r8

.repeat:
    ; Vérifier l'espace de sortie
    cmp r9, r13
    jge .error

    ; output[out_pos++] = value
    mov byte [rdx + r9], r10b
    inc r9
    dec rbx
    jnz .repeat

    jmp .loop

.literal:
    ; Mode literal: copier directement
    cmp r9, r13
    jge .error

    mov byte [rdx + r9], al
    inc r9
    jmp .loop

.error:
    xor rax, rax            ; Retourner 0 en cas d'erreur
    jmp .exit

.done:
    mov rax, r9             ; Retourner taille décompressée

.exit:
    pop r13
    pop r12
    pop rbx
    pop rbp
    ret
