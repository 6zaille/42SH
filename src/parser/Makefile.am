# Déclarer la bibliothèque statique
lib_LIBRARIES = libparser.a

# Ajouter les fichiers source pour la bibliothèque
libparser_a_SOURCES = parser.c parser.h ast.c ast.h

# Options de compilation
libparser_a_CFLAGS = -std=c99 -Werror -Wall -Wextra -Wvla -pedantic
libparser_a_CPPFLAGS = -I$(top_srcdir)/src
