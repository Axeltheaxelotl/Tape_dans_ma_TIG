# Projet woody_woodpacker — Résumé (traduction synthétique)

## But

Coder un "packer" nommé `woody_woodpacker` qui prend en paramètre un exécutable ELF 64 bits (x86_64) et produit un nouveau binaire `woody`. Le binaire `woody` contient le programme chiffré et un loader qui, à l'exécution, affiche `....WOODY....` puis déchiffre et exécute le programme d'origine sans modifier son comportement.

## Comportements attendus

- Nom du programme : `woody_woodpacker` (exécutable fourni par l'étudiant).
- Entrée : chemin d'un fichier binaire ELF 64-bit x86_64.
- Sortie : création d'un fichier exécutable nommé `woody`.
- Lors du packaging (`./woody_woodpacker <binaire>`):
  - Générer une clé de chiffrement aléatoire (la plus aléatoire possible).
  - Afficher la clé sur la sortie standard au format hexadécimal, exemple :
    `key_value: 07A51FF040D45D5CD`.
  - Créer le fichier `woody` contenant le loader + payload chiffré.
- Lors de l'exécution de `./woody` :
  - Afficher exactement la chaîne `....WOODY....` suivie d'un saut de ligne.
  - Déchiffrer le payload en mémoire et exécuter exactement le programme d'origine (aucune modification observable du comportement).
  - Le binaire `woody` ne doit pas planter.

## Contraintes techniques et règles de rendu

- Langage : C (version au choix). Fournir un `Makefile` standard (`all`, `clean`, `fclean`, `re`).
- Gérer uniquement ELF 64-bit (x86_64). Pour toute autre architecture, afficher un message d'erreur clair (ex : "File architecture not suported. x86_64 only").
- Fonctions autorisées (mandatory) : open, close, exit, fpusts, fflush, lseek, mmap, munmap, mprotect, perror, strerror, syscall, fonctions printf, et fonctions de votre `libft` (read, write, malloc, free, ...). Pour la partie obligatoire, se limiter à ces fonctions.
- L'algorithme de chiffrement est libre mais doit être justifié (ROT est considéré trop basique).
- Si un algorithme repose sur une clé, la clé doit être affichée à l'écran et doit être générée aléatoirement.

## Livrables attendus

- Exécutable `woody_woodpacker` (généré via `make`).
- Arborescence source recommandée (exemple) :
  - `src/` (sources C et évent. assembleur)
  - `include/` (headers)
  - `tests/` (programmes exemples + scripts de test)
  - `Makefile`
  - `README.md` (instructions + justification de l'algorithme)
- README détaillant : comment builder, comment tester, algorithme choisi et justification, limitations connues.

## Contrat minimal (entrée / sortie / succès)

- Input : chemin vers un ELF 64-bit exécutable (x86_64).
- Output : création du binaire `woody`.
- Critères de succès :
  - `./woody_woodpacker binaire` imprime `key_value: <HEX>` et crée `woody`.
  - `./woody` imprime `....WOODY....\n` puis exécute le programme d'origine de façon identique (mêmes sorties et même code de retour).
  - Aucune des opérations ne doit provoquer de crash non géré.

## Cas limites à couvrir

- Fichier non exécutable (permissions insuffisantes).
- Fichier non ELF ou ELF 32-bit.
- ELF 64-bit mais architecture différente de x86_64.
- Binaries PIE vs non-PIE (impact sur l'injection/entrypoint).
- Binaries statiques vs dynamiques (relocations, dépendances partagées).
- Très petits exécutables ou problèmes d'alignement/segments.
- Erreurs I/O (mmap, write, open, permissions).

## Recommandations techniques

- Algorithme simple et correct : XOR avec un flux de clés aléatoires (clé de la longueur du payload) — simple à implémenter et suffisant pour la démo. Lecture de `/dev/urandom` pour la clé.
- Pour une meilleure note, envisager AES-CTR/ AES-CTR-like (nécessite plus d'implémentation).
- Méthode d'empaquetage : remplacer `e_entry` par l'offset du loader, ajouter une nouvelle section/segment contenant : loader, payload chiffré, métadonnées (taille, offset, clé si embarquée).
- Loader : afficher `....WOODY....`, déchiffrer en mémoire (avec `mprotect` si nécessaire), puis sauter vers l'entrypoint original.
- Pour la robustesse initiale, on peut refuser les PIE et documenter cette limitation.

## Plan d'implémentation pas-à-pas

1. Analyse initiale et validations : vérifier ELF, classe 64-bit, `e_machine == EM_X86_64`, récupérer `e_entry` et table des segments.
2. Mapper/charger le binaire en mémoire avec `mmap` (ou `read`).
3. Générer la clé : lire `/dev/urandom` pour obtenir assez d'octets.
4. Chiffrer la payload (ex : section .text ou segments exécutables) selon l'algorithme choisi.
5. Construire le binaire `woody` : copier l'original, injecter le loader + données chiffrées, modifier `e_entry`.
6. Écrire `woody` sur disque.
7. Loader embarqué : afficher `....WOODY....\n`, localiser métadonnées, déchiffrer le payload en mémoire, `mprotect` si besoin, sauter à `original_entry`.
8. Tests : vérifier que `./binaire` et `./woody` produisent les mêmes sorties et même code de retour.
9. Gérer les erreurs et documenter les limites.

## Liste d'actions concrètes à réaliser (checklist de développement)

1. Créer l'arborescence du projet (`src/`, `include/`, `tests/`).
2. Implémenter `elf_parser.c` pour valider et lire les headers ELF.
3. Implémenter `keygen.c` lisant `/dev/urandom` et affichant la clé en hex.
4. Implémenter `encrypt.c` (XOR-stream ou AES-CTR selon choix).
5. Implémenter `writer.c` pour créer `woody` et modifier `e_entry`.
6. Implémenter `loader.c` ou `loader.S` embarqué dans `woody`.
7. Écrire `main.c` CLI qui orchestre le tout et affiche `key_value: ...`.
8. Écrire `Makefile` et scripts de tests dans `tests/`.
9. Tester sur plusieurs exécutables simples (Hello World, etc.).
10. Documenter et préparer la soutenance (README + exemples).

---

Fichier créé automatiquement : `SUBJECT_FR.md`.

Si tu veux, je peux maintenant :
- générer automatiquement la structure de fichiers (squelettes `.c`/`.h`, `Makefile`, `README.md`) ; ou
- commencer l'implémentation d'une version minimale (XOR simple, loader embedding clé, non-PIE) et les tests.

Dis-moi quelle option tu préfères.  
