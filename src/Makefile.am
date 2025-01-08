SUBDIRS = execution lexer parser utils

bin_PROGRAMS = 42sh

42sh_SOURCES = main.c

42sh_CPPFLAGS = -I%D%

42sh_CFLAGS = -std=c99 -Werror -Wall -Wextra -Wvla -pedantic

42sh_LDADD = execution/libexecution.a parser/libparser.a lexer/liblexer.a utils/libutils.a
