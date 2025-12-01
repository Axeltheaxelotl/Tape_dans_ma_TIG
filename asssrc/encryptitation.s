
; Fonction : encrypt
; Arguments :
;   rdi = pointeur vers data
;   rsi = taille de data
;   rdx = pointeur vers text
;   rcx = taille de text

encrypt:
    push rbp
    mov rbp, rsp

    xor r8, r8        ; r8 = index pour parcourir data
    xor r9, r9        ; r9 = clé (valeur ajoutée à chaque tour)

main_loop:
    cmp r8, rsi       ; Si index >= taille de data, on passe à la suite
    je update_key

    mov al, [rdi + r8]    ; Charger data[index] dans al
    add al, r9b           ; Ajouter la clé courante à al
    xor [rdx + r8], al    ; XOR text[index] avec al (modifie text)
    inc r8                ; index++
    jmp main_loop         ; Recommencer la boucle

update_key:
    add r9b, 42           ; Incrémenter la clé (r9b) de 42
    xor r8, r8            ; Remettre l'index à 0
    loop main_loop        ; rcx-- et recommence si rcx != 0

    pop rbp
    ret