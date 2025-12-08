# Tests de Compression RLE pour Woody Woodpacker

## 🎯 Objectif

Ce système de tests valide que l'algorithme de compression RLE (Run-Length Encoding) fonctionne correctement pour woody_woodpacker, tant en compression (C) qu'en décompression (C + Assembly).

## 📋 Comment utiliser

### Lancer les tests

```bash
make test-rle
```

Cette commande va :
1. Compiler le code de compression (`src/compression/rle_compress.c`)
2. Assembler le code de décompression (`asssrc/rle_decompress_64.s`)
3. Compiler le programme de test (`rle_standalone.c`)
4. Exécuter automatiquement tous les tests
5. Afficher les résultats

### Nettoyer les fichiers de test

```bash
make test-rle-clean
```

## 🧪 Tests inclus

### Test 1 : Répétitions simples
- Teste des séquences basiques de bytes répétés
- Vérifie la compression et décompression de base

### Test 2 : Simulation padding .bss (512 zéros)
- Simule une section .bss (initialisée à zéro)
- Doit donner un excellent ratio de compression (~98%)
- Cas d'usage typique pour woody

### Test 3 : Code avec NOPs et padding
- Simule du code assembleur avec NOPs d'alignement
- Cas réel lors de l'encryption de binaires

### Test 4 : Worst case (données aléatoires)
- Teste le pire scénario (aucune répétition)
- Vérifie que la compression n'est pas rentable
- Important pour décider si on active RLE ou non

### Test 5 : Bytes avec bit 7 set (0x80-0xFF)
- Teste les bytes >= 128 qui nécessitent un traitement spécial
- Critique car le format RLE utilise le bit 7

### Test 6 : Longue séquence (> 127 répétitions)
- Teste les séquences dépassant la limite de 127
- Vérifie la gestion des chunks multiples

### Test 7 : Cas limites
- Buffer NULL
- 1 seul byte
- Cas edge pour robustesse

## 📊 Résultats attendus

Tous les tests doivent passer (7/7) avec :
- ✅ Compression C fonctionnelle
- ✅ Décompression C fonctionnelle
- ✅ Décompression ASM fonctionnelle
- ✅ Identité parfaite entre original et décompressé

## 🔧 Format RLE utilisé

### Compression
- **Mode RLE** : `[count|0x80][value]` pour répétitions ≥ 3
- **Mode Literal** : `[value]` pour bytes < 128 sans répétition
- **Mode Literal forcé** : `[0x81][value]` pour bytes ≥ 128 sans répétition

### Contraintes
- Count maximum : 127 (0x7F)
- Bit 7 = 1 indique mode RLE
- Bit 7 = 0 indique literal

## 💡 Intégration dans woody

### 1. Vérifier si compression rentable

```c
size_t original_size = section_size;
size_t compressed_size = compression(data, original_size, compressed_buffer, max_size);

if (rle_is_worth_it(original_size, compressed_size)) {
    // Utiliser la version compressée
    // Injecter le stub de décompression ASM
} else {
    // Ne pas compresser
}
```

### 2. Injecter le stub de décompression

Le fichier `asssrc/rle_decompress_64.s` contient le code assembly à injecter dans le binaire woody.

Au démarrage de woody, appeler :
```asm
call rle_decompress_stub_64
```

### 3. Fonctions disponibles

```c
// Compression (en C, à utiliser par woody_woodpacker)
size_t compression(const unsigned char *input, size_t taille_input, 
                   unsigned char *output, size_t max_output);

// Décompression C (pour tests)
size_t decompression(const unsigned char *input, size_t taille_input, 
                     unsigned char *output, size_t max_output);

// Décompression ASM (pour woody au runtime)
extern size_t rle_decompress_stub_64(const unsigned char *input, size_t taille_input,
                                      unsigned char *output, size_t max_output);

// Vérifier si compression rentable (gain ≥ 10%)
int rle_is_worth_it(size_t taille_original, size_t taille_compressed);
```

## 🐛 Debugging

Si des tests échouent :

1. **Vérifier la compilation** : Les erreurs de syntaxe sont affichées
2. **Comparer les hex dumps** : Les bytes sont affichés pour chaque étape
3. **Vérifier la cohérence C vs ASM** : Les deux décompressions doivent donner le même résultat
4. **Ratio de compression** : Doit être < 100% pour les cas favorables

## ✅ Prochaines étapes

Une fois les tests RLE validés :

1. Intégrer la compression dans `woody_woodpacker`
2. Modifier l'injection pour inclure le stub ASM
3. Tester sur de vrais binaires (32 et 64 bits)
4. Mesurer le gain de taille réel
5. Valider que woody fonctionne correctement après décompression

## 📝 Notes importantes

- La compression RLE est un **BONUS** du projet woody
- Elle est particulièrement efficace pour les sections .bss et les NOPs
- Le gain typique est de 50-98% sur les sections avec répétitions
- Sur du code dense sans répétitions, ne pas activer la compression
- L'assembly est recommandé pour les performances dans woody

---

**Auteur** : Tests pour woody_woodpacker - Projet 42  
**Date** : Décembre 2025
