# Dockerfile multi-stage pour tester woody_woodpacker avec ELF et PE
FROM ubuntu:22.04 AS builder

# Éviter les prompts interactifs
ENV DEBIAN_FRONTEND=noninteractive

# Installation des dépendances de build
RUN apt-get update && apt-get install -y \
    gcc \
    g++ \
    make \
    nasm \
    mingw-w64 \
    gcc-multilib \
    && rm -rf /var/lib/apt/lists/*

WORKDIR /workspace

# Copier tous les fichiers du projet
COPY . .

# Compiler woody_woodpacker
RUN make re

# Compiler des binaires de test ELF
RUN gcc docker/test_binaries/test_elf.c -o docker/test_binaries/test_elf64 || true
RUN gcc -m32 docker/test_binaries/test_elf.c -o docker/test_binaries/test_elf32 2>/dev/null || true

# Compiler des binaires de test PE
RUN cd docker/test_binaries && \
    x86_64-w64-mingw32-gcc test_pe.c -o test_pe64.exe && \
    i686-w64-mingw32-gcc test_pe.c -o test_pe32.exe 2>/dev/null || true

# Stage 2: Environnement de test avec Wine pour PE
FROM ubuntu:22.04

ENV DEBIAN_FRONTEND=noninteractive

    # Installation de Wine et des outils nécessaires
RUN dpkg --add-architecture i386 && \
    apt-get update && \
    apt-get install -y \
    wine \
    wine32 \
    wine64 \
    mingw-w64 \
    gcc \
    gcc-multilib \
    make \
    nasm \
    file \
    bsdmainutils \
    xxd \
    strace \
    binutils \
    && rm -rf /var/lib/apt/lists/*WORKDIR /workspace

# Copier tout le projet
COPY --from=builder /workspace .

# Créer des répertoires pour les tests
RUN mkdir -p docker/test_results

# Initialiser Wine
ENV WINEARCH=win64
ENV WINEPREFIX=/root/.wine64
RUN wineboot -u 2>/dev/null || true

# Rendre les scripts exécutables
RUN chmod +x docker/*.sh 2>/dev/null || true
RUN chmod +x docker/test_binaries/*.sh 2>/dev/null || true

# Point d'entrée par défaut
ENTRYPOINT ["/bin/bash"]
CMD ["docker/test_all.sh"]
