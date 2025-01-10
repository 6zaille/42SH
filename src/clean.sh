make clean;

rm ../autom4te.cache -rf;
rm ../aclocal.m4;
rm ../ar-lib;
rm ../compile;
rm ../config.log;
rm ../configure;
rm ../depcomp;
rm ../install-sh;
rm ../missing;
rm ../Makefile.in;
rm ../Makefile;
rm ../configure~;
rm Makefile.in;
rm Makefile

rm execution/libexecution.a;
rm execution/libexecution_a-*.o;
rm execution/Makefile.in;
rm execution/Makefile;
rm -rf execution/.deps;

rm lexer/liblexer.a;
rm lexer/liblexer_a-*.o;
rm lexer/Makefile.in;
rm lexer/Makefile;
rm -rf lexer/.deps;

rm parser/libparser.a;
rm parser/libparser_a-*.o;
rm parser/Makefile.in;
rm parser/Makefile;
rm -rf parser/.deps;

rm utils/libutils.a;
rm utils/libutils_a-*.o;
rm utils/Makefile.in;
rm utils/Makefile;
rm -rf utils/.deps;


rm 42sh;
rm 42sh-main.o;
