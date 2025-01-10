#include "builtins.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int builtin_echo(int argc, char **argv)
{
    int newline = 1;
    int start = 1;

    if (argc > 1 && strcmp(argv[1], "-n") == 0)
    {
        newline = 0;
        start = 2;
    }

    for (int i = start; i < argc; i++)
    {
        printf("%s", argv[i]);
        if (i < argc - 1)
        {
            printf(" ");
        }
    }
    if (newline)
    {
        printf("\n");
    }
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