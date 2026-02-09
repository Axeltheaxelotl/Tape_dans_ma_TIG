# Test PE - Analyseur de fichiers PE pour Linux

## Description

`pe_test` est un utilitaire simple pour analyser des fichiers PE (Portable Executable) directement sur Linux, sans avoir besoin de les exécuter. C'est pratique pour tester le parser PE du projet woody_woodpacker.

## Compilation

```bash
make pe_test
```

## Utilisation

```bash
./pe_test <fichier.exe>
```

### Exemple
```bash
# Télécharger un fichier PE de test
wget https://github.com/corkami/pocs/raw/master/PE/bin/tiny.exe

# Analyser le fichier
./pe_test tiny.exe
```

## Ce qu'il affiche

- ✅ **Informations basiques** : taille, architecture (32/64), point d'entrée
- ✅ **Table des sections** : nom, RVA, taille, permissions (rwx)
- ✅ **Détails .text** : section code avec dump hexadécimal
- ✅ **Tests de validation** : signature DOS, section .text, conversions RVA

## Obtenir des fichiers PE de test

### Méthode 1 : Compiler avec MinGW (si installé)
```bash
# Installer mingw-w64 sur Linux
sudo apt install mingw-w64  # Debian/Ubuntu
# ou
sudo dnf install mingw64-gcc  # Fedora

# Compiler un test simple
cat > test.c << 'EOF'
#include <stdio.h>
int main() {
    printf("Hello Windows!\n");
    return 0;
}
EOF

i686-w64-mingw32-gcc test.c -o test32.exe    # 32-bit
x86_64-w64-mingw32-gcc test.c -o test64.exe  # 64-bit
```

### Méthode 2 : Télécharger des samples
```bash
# PE tiny (très petit, bon pour les tests)
wget https://github.com/corkami/pocs/raw/master/PE/bin/tiny.exe

# Autres samples de test
git clone https://github.com/corkami/pocs.git
ls pocs/PE/bin/*.exe
```

### Méthode 3 : Utiliser Docker avec Wine
```bash
# Voir les scripts dans docker/ pour compiler des PE
cd docker/test_binaries
./compile.sh  # si disponible
```

## Nettoyage

```bash
make clean-tests  # Supprime pe_test et autres binaires de test
```

## Sortie exemple

```
🔍 Analyse du fichier: sample.exe

╔══════════════════════════════════════════════════╗
║          Informations du fichier PE             ║
╚══════════════════════════════════════════════════╝

📦 Taille fichier:    2048 octets
🔧 Architecture:      PE32+ (64-bit)
🎯 Point d'entrée:    0x00001000 (RVA)
📂 Nombre sections:   3

╔══════════════════════════════════════════════════╗
║                    Sections                      ║
╚══════════════════════════════════════════════════╝

  Nom         RVA         VirtSize    RawSize     Perms
  ───────────────────────────────────────────────────────────
  .text       0x00001000  0x00000200  0x00000200  r-x
  .data       0x00002000  0x00000100  0x00000100  rw-
  .rdata      0x00003000  0x00000100  0x00000100  r--

✨ Analyse terminée avec succès!
```
