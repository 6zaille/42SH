#!/bin/bash

# Assurez-vous que le script s'exécute depuis son répertoire
cd "$(dirname "$0")"

# Nettoyage des fichiers générés par make
make clean;

# Suppression des fichiers générés par autotools
rm -rf autom4te.cache aclocal.m4 ar-lib compile config.log configure depcomp \
       install-sh missing Makefile.in Makefile;

# Suppression des artefacts spécifiques dans chaque module
for dir in src/execution src/lexer src/parser src/utils; do
    echo "Suppression dans le dossier : $dir"
    rm -rf $dir/.deps $dir/Makefile.in $dir/Makefile $dir/*.o $dir/*.a
done

rm -rf .vscode;
rm config.status;
rm -rf src/.deps;
rm configure~;  
rm src/Makefile.in src/Makefile;
# Suppression du binaire et des fichiers intermédiaires principaux
rm -f src/42sh 42sh-main.o;
rm clean;
