bin_PROGRAMS = 42sh

42sh_SOURCES = src/main.c src/utils.c src/parser.c src/lexer.c src/executor.c

SUBDIRS = src tests

CLEANFILES = Makefile.in aclocal.m4 autom4te.cache config.h.in \
             configure depcomp install-sh missing config.log config.status

distclean-local:
    rm -rf config.h config.log config.status autom4te.cache

check_SCRIPTS = tests/test_suite.sh
TESTS = $(check_SCRIPTS)