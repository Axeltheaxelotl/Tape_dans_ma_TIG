BITS 32

global rle_decompress_stub_32

; Fonction: rle_decompress_stub_32
; Arguments (cdecl 32-bit):
;   [ebp+8]  = compressed data (pointeur)
;   [ebp+12] = compressed size
;   [ebp+16] = output buffer (pointeur)
;   [ebp+20] = output max size
; Retour:
;   eax = taille décompressée, ou 0 si erreur

rle_decompress_stub_32:
    push ebp
    mov ebp, esp
    push ebx
    push esi
    push edi
    push ecx

    ; Charger les arguments
    mov esi, [ebp + 8]    ; esi = compressed data
    mov ecx, [ebp + 12]   ; ecx = compressed size
    mov edi, [ebp + 16]   ; edi = output buffer
    
    xor ebx, ebx          ; ebx = in_pos (index dans compressed)
    xor eax, eax          ; eax = out_pos (index dans output)

.loop:
    cmp ebx, ecx          ; if (in_pos >= compressed_size)
    jge .done

    ; Lire le byte
    movzx edx, byte [esi + ebx]
    inc ebx

    test dl, 0x80         ; Tester bit 7
    jz .literal

.rle_mode:
    ; Mode RLE: [count|0x80][value]
    and edx, 0x7F         ; count = byte & 0x7F
    push edx              ; Sauvegarder count
    
    cmp ebx, ecx          ; Vérifier qu'on peut lire value
    jge .error_with_stack

    movzx edx, byte [esi + ebx]  ; edx = value
    inc ebx
    
    pop ecx               ; ecx = count
    push ecx              ; Remettre count sur la pile
    
.repeat:
    ; Vérifier l'espace de sortie
    cmp eax, [ebp + 20]
    jge .error_with_stack

    ; output[out_pos++] = value
    mov byte [edi + eax], dl
    inc eax
    dec ecx
    jnz .repeat

    pop ecx               ; Nettoyer la pile (count)
    mov ecx, [ebp + 12]   ; Restaurer compressed_size
    jmp .loop

.literal:
    ; Mode literal: copier directement
    cmp eax, [ebp + 20]
    jge .error

    mov byte [edi + eax], dl
    inc eax
    jmp .loop

.error_with_stack:
    pop ecx               ; Nettoyer la pile
.error:
    xor eax, eax          ; Retourner 0 en cas d'erreur
    jmp .exit

.done:
    ; eax contient déjà out_pos

.exit:
    pop ecx
    pop edi
    pop esi
    pop ebx
    pop ebp
    ret
