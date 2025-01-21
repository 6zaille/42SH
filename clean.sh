#!/bin/bash

cd "$(dirname "$0")"

make clean;

rm -rf autom4te.cache aclocal.m4 ar-lib compile config.log configure depcomp \
       install-sh missing Makefile.in Makefile;

for dir in src/execution src/lexer src/parser src/utils; do
    echo "Suppression dans le dossier : $dir"
    rm -rf $dir/.deps $dir/Makefile.in $dir/Makefile $dir/*.o $dir/*.a
done

rm -rf .vscode;
rm config.status;
rm -rf src/.deps;
rm configure~;
rm src/Makefile.in src/Makefile;
rm test_script.sh;
rm -rf src/42sh
rm -rf src/42sh-main.o;
rm tests/Makefile.in;
rm tests/Makefile;
rm test-driver;
rm file.txt;
rm input.txt;
rm output.txt;
rm out;
rm clean;
clear;
echo -e "🐒🐒🐒🐒🐒 XingXing a mangé toutes les trash files 🐒🐒🐒🐒🐒"
