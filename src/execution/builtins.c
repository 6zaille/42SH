#include "builtins.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int builtin_echo(int argc, char **argv) {
    int newline = 1;
    int interpret_escapes = 0;
    int i = 1;

    while (i < argc && argv[i][0] == '-') {
        if (strcmp(argv[i], "-n") == 0)
            newline = 0;
        else if (strcmp(argv[i], "-e") == 0)
            interpret_escapes = 1;
        else if (strcmp(argv[i], "-E") == 0)
            interpret_escapes = 0;
        else
            break;
        i++;
    }

    for (; i < argc; i++) {
        if (interpret_escapes) {
            for (char *p = argv[i]; *p; p++) {
                if (*p == '\\') {
                    p++;
                    switch (*p) {
                        case 'n': putchar('\n'); break;
                        case 't': putchar('\t'); break;
                        case '\\': putchar('\\'); break;
                        default: putchar('\\'); putchar(*p); break;
                    }
                } else {
                    putchar(*p);
                }
            }
        } else {
            printf("%s", argv[i]);
        }

        if (i < argc - 1)
            putchar(' ');
    }

    if (newline)
        putchar('\n');

    fflush(stdout);
    return 0;
}

int builtin_true(void)
{
    return 0;
}

int builtin_false(void)
{
    return 1;
}

int builtin_exit(int argc, char **argv)
{
    int exit_code = 0;

    if (argc > 1)
    {
        exit_code = atoi(argv[1]);
    }

    exit(exit_code);
    return 0; // Jamais atteint mais pour M-CM-)viter des warnings.
}
