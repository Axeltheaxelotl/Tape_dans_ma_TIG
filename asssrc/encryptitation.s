
; Fonction : encrypt
; Arguments :
;   rdi = pointeur vers key
;   rsi = taille de key
;   rdx = pointeur vers text
;   rcx = taille de text

global encrypt
global encryptitation

encrypt:
encryptitation:
    push rbp
    mov rbp, rsp

    xor r8, r8        ; r8 = index clé (key_index)
    xor r9, r9        ; r9 = offset additionnel
    xor r10, r10      ; r10 = index texte (text_index)

main_loop:
    cmp r10, rcx      ; Si text_index >= taille de text, on a fini
    jge done

    mov al, [rdi + r8]    ; Charger key[key_index] dans al
    add al, r9b           ; Ajouter l'offset
    xor [rdx + r10], al   ; XOR text[text_index] avec al
    
    inc r10               ; text_index++
    inc r8                ; key_index++
    
    cmp r8, rsi           ; Si key_index >= key_size
    jl main_loop          ; Continue si key_index < key_size
    
    add r9b, 42           ; offset += 42
    xor r8, r8            ; key_index = 0
    jmp main_loop

done:
    pop rbp
    ret