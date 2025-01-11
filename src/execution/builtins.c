#include "builtins.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void print_escaped_char(char c) {
    switch (c) {
        case 'n': 
            putchar('\n'); 
            break;
        case 't':
            putchar('\t'); 
            break;
        case '\\':
            putchar('\\');
            break;
        default:
            putchar('\\');
            putchar(c);
            break;
    }
}

void print_argument(char *arg, int interpret_escapes) {
    if (interpret_escapes) {
        for (char *p = arg; *p; p++) {
            if (*p == '\\') {
                p++;
                print_escaped_char(*p);
            } else {
                putchar(*p);
            }
        }
    } else {
        printf("%s", arg);
    }
}

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
        print_argument(argv[i], interpret_escapes);
        if (i < argc - 1)
            putchar(' ');
    }

    if (newline)
        putchar('\n');

    fflush(stdout);
    return 0;
}

int builtin_true(void) {
    return 0;
}

int builtin_false(void) {
    return 1;
}

int builtin_exit(int argc, char **argv) {
    int exit_code = 0;

    if (argc > 1) {
        exit_code = atoi(argv[1]);
    }

    exit(exit_code);
    return 0; // Never reached but avoids warnings.
}
