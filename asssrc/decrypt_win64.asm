; Stub de déchiffrement pour PE Windows 64-bit
; Ce code sera injecté dans une nouvelle section .woody du PE

BITS 64

section .text
    global _start

_start:
    ; Sauvegarder les registres
    push rax
    push rcx
    push rdx
    push r8
    push r9
    push r10
    push r11
    
    ; Afficher "....WOODY...."
    ; Windows x64 calling convention: RCX, RDX, R8, R9
    sub rsp, 40                     ; Shadow space (32) + alignment (8)
    
    ; GetStdHandle(STD_OUTPUT_HANDLE = -11)
    mov rcx, -11                    ; STD_OUTPUT_HANDLE
    call [rel GetStdHandle_ptr]
    mov r10, rax                    ; Sauver le handle dans r10
    
    ; WriteConsole(handle, buffer, length, written, NULL)
    mov rcx, r10                    ; hConsoleOutput
    lea rdx, [rel woody_msg]        ; lpBuffer
    mov r8, 14                      ; nNumberOfCharsToWrite
    lea r9, [rel bytes_written]     ; lpNumberOfCharsWritten
    mov qword [rsp+32], 0           ; lpReserved = NULL
    call [rel WriteConsoleA_ptr]
    
    ; Déchiffrer la section .text
    lea rsi, [rel text_start]       ; Adresse de début de .text (à patcher)
    mov rcx, 0                      ; Taille de .text (à patcher)
    lea rdi, [rel key_data]         ; Clé de déchiffrement
    mov r8, 32                      ; Taille de la clé (256 bits = 32 bytes)
    xor r9, r9                      ; Index de clé
    xor r11, r11                    ; Offset additionnel
    
decrypt_loop:
    test rcx, rcx
    jz decrypt_done
    
    ; XOR byte avec la clé
    mov al, byte [rdi + r9]         ; Récupérer byte de clé
    add al, r11b                    ; Ajouter offset
    xor byte [rsi], al              ; XOR avec le byte de texte
    
    inc rsi                         ; Prochain byte de texte
    inc r9                          ; Prochain byte de clé
    dec rcx                         ; Décrémenter compteur
    
    ; Reset key index si fin de clé
    cmp r9, r8
    jne skip_reset
    xor r9, r9
    add r11b, 42                    ; Modifier offset pour variabilité
    
skip_reset:
    jmp decrypt_loop
    
decrypt_done:
    add rsp, 40                     ; Nettoyer shadow space
    
    ; Restaurer les registres
    pop r11
    pop r10
    pop r9
    pop r8
    pop rdx
    pop rcx
    pop rax
    
    ; Sauter vers le vrai entry point (à patcher)
    jmp original_entry_point
    
; Données
align 8
woody_msg:      db "....WOODY....", 0x0A
bytes_written:  dq 0

; Pointeurs vers API Windows (à résoudre dynamiquement ou patcher)
GetStdHandle_ptr:   dq 0
WriteConsoleA_ptr:  dq 0

; Données de déchiffrement (à patcher lors de l'injection)
align 16
text_start:             dq 0    ; RVA de .text
text_size:              dq 0    ; Taille de .text
original_entry_point:   dq 0    ; Original entry point RVA

align 32
key_data:               times 32 db 0    ; Clé de 256 bits

section_end:
