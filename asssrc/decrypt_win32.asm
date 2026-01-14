; Stub de déchiffrement pour PE Windows 32-bit
; Ce code sera injecté dans une nouvelle section .woody du PE

BITS 32

section .text
    global _start

_start:
    ; Sauvegarder les registres
    pushad
    pushfd
    
    ; Afficher "....WOODY...."
    ; Windows 32-bit: arguments sur la pile (stdcall)
    
    ; GetStdHandle(STD_OUTPUT_HANDLE = -11)
    push -11                        ; STD_OUTPUT_HANDLE
    call [GetStdHandle_ptr]
    mov edi, eax                    ; Sauver le handle
    
    ; WriteConsoleA(handle, buffer, length, written, NULL)
    push 0                          ; lpReserved = NULL
    push bytes_written              ; lpNumberOfCharsWritten
    push 14                         ; nNumberOfCharsToWrite
    push woody_msg                  ; lpBuffer
    push edi                        ; hConsoleOutput
    call [WriteConsoleA_ptr]
    
    ; Déchiffrer la section .text
    mov esi, [text_start]           ; Adresse de début de .text
    mov ecx, [text_size]            ; Taille de .text
    mov edi, key_data               ; Clé de déchiffrement
    mov edx, 32                     ; Taille de la clé
    xor ebx, ebx                    ; Index de clé
    xor ah, ah                      ; Offset additionnel
    
decrypt_loop:
    test ecx, ecx
    jz decrypt_done
    
    ; XOR byte avec la clé
    mov al, byte [edi + ebx]        ; Récupérer byte de clé
    add al, ah                      ; Ajouter offset
    xor byte [esi], al              ; XOR avec le byte de texte
    
    inc esi                         ; Prochain byte de texte
    inc ebx                         ; Prochain byte de clé
    dec ecx                         ; Décrémenter compteur
    
    ; Reset key index si fin de clé
    cmp ebx, edx
    jne skip_reset
    xor ebx, ebx
    add ah, 42                      ; Modifier offset pour variabilité
    
skip_reset:
    jmp decrypt_loop
    
decrypt_done:
    ; Restaurer les registres
    popfd
    popad
    
    ; Sauter vers le vrai entry point
    jmp [original_entry_point]
    
; Données
align 4
woody_msg:      db "....WOODY....", 0x0A
bytes_written:  dd 0

; Pointeurs vers API Windows (à patcher lors de l'injection)
GetStdHandle_ptr:   dd 0
WriteConsoleA_ptr:  dd 0

; Données de déchiffrement (à patcher lors de l'injection)
align 4
text_start:             dd 0    ; RVA de .text
text_size:              dd 0    ; Taille de .text
original_entry_point:   dd 0    ; Original entry point RVA

align 32
key_data:               times 32 db 0    ; Clé de 256 bits

section_end:
