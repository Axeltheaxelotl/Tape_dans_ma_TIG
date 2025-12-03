; Simple 32-bit program for testing woody
BITS 32

section .data
    msg db "Hello from 32-bit!", 10
    len equ $ - msg

section .text
    global _start

_start:
    ; write(1, msg, len)
    mov eax, 4          ; sys_write
    mov ebx, 1          ; stdout
    mov ecx, msg
    mov edx, len
    int 0x80

    ; exit(0)
    mov eax, 1          ; sys_exit
    xor ebx, ebx        ; status 0
    int 0x80
