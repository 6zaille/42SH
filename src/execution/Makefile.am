# Déclarer la bibliothèque statique
lib_LIBRARIES = libexecution.a

# Ajouter les fichiers source pour la bibliothèque
liblexer_a_SOURCES = builtins.c builtins.h exec.c exec.h

# Options de compilation
liblexer_a_CFLAGS = -std=c99 -Werror -Wall -Wextra -Wvla -pedantic
liblexer_a_CPPFLAGS = -I$(top_srcdir)/src
