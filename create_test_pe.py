#!/usr/bin/env python3
"""
Générateur de fichiers PE minimaux pour tester woody_woodpacker
Crée des PE 32 et 64 bits valides avec section .text
"""

import struct
import sys

def create_pe32(filename):
    """Crée un PE 32-bit minimal"""
    
    # DOS Header (64 bytes)
    dos_header = bytearray(64)
    dos_header[0:2] = b'MZ'  # e_magic
    dos_header[60:64] = struct.pack('<I', 0x80)  # e_lfanew (offset to PE header)
    
    # DOS Stub (32 bytes) - "This program cannot be run in DOS mode"
    dos_stub = b'This program cannot be run in DOS mode.\r\r\n$'
    dos_stub += b'\x00' * (32 - len(dos_stub))
    
    # PE Signature (4 bytes)
    pe_sig = b'PE\x00\x00'
    
    # COFF File Header (20 bytes)
    coff_header = struct.pack('<HHIIIHH',
        0x014C,      # Machine (i386)
        1,           # NumberOfSections
        0x12345678,  # TimeDateStamp
        0,           # PointerToSymbolTable
        0,           # NumberOfSymbols
        224,         # SizeOfOptionalHeader (PE32)
        0x010F       # Characteristics (EXECUTABLE_IMAGE | LINE_NUMS_STRIPPED | ...)
    )
    
    # Optional Header PE32 (224 bytes)
    opt_header = struct.pack('<HHBBIIIIIIIHHHHHHIIHHHHHHIIIIHHIIIIII',
        0x010B,         # Magic (PE32)
        14, 0,          # Linker version
        0x200,          # SizeOfCode
        0,              # SizeOfInitializedData
        0,              # SizeOfUninitializedData
        0x1000,         # AddressOfEntryPoint
        0x1000,         # BaseOfCode
        0x2000,         # BaseOfData
        0x00400000,     # ImageBase
        0x1000,         # SectionAlignment
        0x200,          # FileAlignment
        5, 1,           # OS version
        0, 0,           # Image version
        5, 1,           # Subsystem version
        0,              # Win32VersionValue
        0x3000,         # SizeOfImage
        0x200,          # SizeOfHeaders
        0,              # CheckSum
        3,              # Subsystem (Console)
        0,              # DllCharacteristics
        0x100000,       # SizeOfStackReserve
        0x1000,         # SizeOfStackCommit
        0x100000,       # SizeOfHeapReserve
        0x1000,         # SizeOfHeapCommit
        0,              # LoaderFlags
        16              # NumberOfRvaAndSizes
    )
    
    # Data directories (16 entries * 8 bytes = 128 bytes)
    data_dirs = b'\x00' * 128
    
    # Section Header .text (40 bytes)
    section_header = bytearray(40)
    section_header[0:6] = b'.text\x00'  # Name
    section_header[8:12] = struct.pack('<I', 0x100)    # VirtualSize
    section_header[12:16] = struct.pack('<I', 0x1000)  # VirtualAddress
    section_header[16:20] = struct.pack('<I', 0x200)   # SizeOfRawData
    section_header[20:24] = struct.pack('<I', 0x200)   # PointerToRawData
    section_header[36:40] = struct.pack('<I', 0x60000020)  # Characteristics (CODE | EXECUTE | READ)
    
    # Shellcode: mov eax, 42; push eax; push 0; call ExitProcess; int3
    shellcode = bytes([
        0xB8, 0x2A, 0x00, 0x00, 0x00,  # mov eax, 42
        0x50,                          # push eax
        0x6A, 0x00,                    # push 0
        0xC3,                          # ret (ou int3)
        0xCC                           # int3
    ])
    
    # Code section (0x200 bytes aligned)
    code_section = shellcode + b'\x00' * (0x200 - len(shellcode))
    
    # Assemblage du fichier
    with open(filename, 'wb') as f:
        f.write(dos_header)
        f.write(dos_stub)
        f.write(pe_sig)
        f.write(coff_header)
        f.write(opt_header)
        f.write(data_dirs)
        f.write(section_header)
        
        # Padding jusqu'à 0x200
        current_pos = len(dos_header) + len(dos_stub) + len(pe_sig) + len(coff_header) + len(opt_header) + len(data_dirs) + len(section_header)
        padding = b'\x00' * (0x200 - current_pos)
        f.write(padding)
        
        # Code section
        f.write(code_section)
    
    print(f"✅ PE32 créé: {filename}")
    print(f"   Taille: {0x400} octets")
    print(f"   Entry Point: 0x1000 (RVA)")
    print(f"   Section .text: RVA=0x1000, Size=0x200")


def create_pe64(filename):
    """Crée un PE 64-bit minimal"""
    
    # DOS Header (64 bytes)
    dos_header = bytearray(64)
    dos_header[0:2] = b'MZ'
    dos_header[60:64] = struct.pack('<I', 0x80)
    
    # DOS Stub (32 bytes)
    dos_stub = b'This program cannot be run in DOS mode.\r\r\n$'
    dos_stub += b'\x00' * (32 - len(dos_stub))
    
    # PE Signature (4 bytes)
    pe_sig = b'PE\x00\x00'
    
    # COFF File Header (20 bytes)
    coff_header = struct.pack('<HHIIIHH',
        0x8664,      # Machine (x64)
        1,           # NumberOfSections
        0x12345678,  # TimeDateStamp
        0,           # PointerToSymbolTable
        0,           # NumberOfSymbols
        240,         # SizeOfOptionalHeader (PE32+)
        0x0022       # Characteristics
    )
    
    # Optional Header PE32+ (240 bytes)
    opt_header = struct.pack('<HHBBIIIIQIIHHHHHHIIHHHHHHQQQQIIHHIIIIII',
        0x020B,         # Magic (PE32+)
        14, 0,          # Linker version
        0x200,          # SizeOfCode
        0,              # SizeOfInitializedData
        0,              # SizeOfUninitializedData
        0x1000,         # AddressOfEntryPoint
        0x1000,         # BaseOfCode
        0x0000000140000000,  # ImageBase (8 bytes)
        0x1000,         # SectionAlignment
        0x200,          # FileAlignment
        6, 0,           # OS version
        0, 0,           # Image version
        6, 0,           # Subsystem version
        0,              # Win32VersionValue
        0x3000,         # SizeOfImage
        0x200,          # SizeOfHeaders
        0,              # CheckSum
        3,              # Subsystem (Console)
        0,              # DllCharacteristics
        0x100000,       # SizeOfStackReserve (8 bytes)
        0x1000,         # SizeOfStackCommit (8 bytes)
        0x100000,       # SizeOfHeapReserve (8 bytes)
        0x1000,         # SizeOfHeapCommit (8 bytes)
        0,              # LoaderFlags
        16              # NumberOfRvaAndSizes
    )
    
    # Data directories
    data_dirs = b'\x00' * 128
    
    # Section Header .text
    section_header = bytearray(40)
    section_header[0:6] = b'.text\x00'
    section_header[8:12] = struct.pack('<I', 0x100)
    section_header[12:16] = struct.pack('<I', 0x1000)
    section_header[16:20] = struct.pack('<I', 0x200)
    section_header[20:24] = struct.pack('<I', 0x200)
    section_header[36:40] = struct.pack('<I', 0x60000020)
    
    # Shellcode 64-bit: mov rcx, 42; sub rsp, 0x28; ret
    shellcode = bytes([
        0x48, 0xC7, 0xC1, 0x2A, 0x00, 0x00, 0x00,  # mov rcx, 42
        0x48, 0x83, 0xEC, 0x28,                    # sub rsp, 0x28
        0xC3,                                      # ret
        0xCC                                       # int3
    ])
    
    # Code section
    code_section = shellcode + b'\x00' * (0x200 - len(shellcode))
    
    # Assemblage
    with open(filename, 'wb') as f:
        f.write(dos_header)
        f.write(dos_stub)
        f.write(pe_sig)
        f.write(coff_header)
        f.write(opt_header)
        f.write(data_dirs)
        f.write(section_header)
        
        current_pos = len(dos_header) + len(dos_stub) + len(pe_sig) + len(coff_header) + len(opt_header) + len(data_dirs) + len(section_header)
        padding = b'\x00' * (0x200 - current_pos)
        f.write(padding)
        
        f.write(code_section)
    
    print(f"✅ PE64 créé: {filename}")
    print(f"   Taille: {0x400} octets")
    print(f"   Entry Point: 0x1000 (RVA)")
    print(f"   Section .text: RVA=0x1000, Size=0x200")


def main():
    print("\n🔨 Génération de fichiers PE de test...\n")
    
    # Créer les deux versions
    create_pe32("test32.exe")
    print()
    create_pe64("test64.exe")
    
    print("\n✨ Fichiers créés avec succès!")
    print("\n📝 Vous pouvez maintenant les tester:")
    print("   ./pe_test test32.exe")
    print("   ./pe_test test64.exe")
    print("   ./woody_woodpacker test32.exe")
    print("   ./woody_woodpacker test64.exe\n")


if __name__ == '__main__':
    main()
