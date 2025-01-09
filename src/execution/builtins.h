#ifndef BUILTINS_H
#define BUILTINS_H

int builtin_echo(int argc, char **argv);
int builtin_true(void);
int builtin_false(void);
int builtin_exit(int argc, char **argv);

#endif /* !BUILTINS_H */
