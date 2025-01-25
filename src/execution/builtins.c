#include "builtins.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "../parser/parser.h"

void print_with_escape(const char *str)
{
    while (*str)
    {
        if (*str == '\\')
        {
            str++;
            switch (*str)
            {
            case 'n':
                putchar('n');
                break;
            case 't':
                putchar('t');
                break;
            case '\\':
                putchar('\\');
                break;
            default:
                putchar('\\');
                putchar(*str);
                break;
            }
        }
        else
        {
            putchar(*str);
        }
        str++;
    }
}

int builtin_echo(int argc, char **argv)
{
    int flag_n = 0;
    int flag_e = 0; // -e enabled
    int flag_E = 1; // -E enabled by default
    int i = 1;

    // Traitement des options
    while (i < argc && argv[i][0] == '-' && argv[i][1] != '\0')
    {
        for (int j = 1; argv[i][j] != '\0'; j++)
        {
            if (argv[i][j] == 'n')
                flag_n = 1;
            else if (argv[i][j] == 'e')
            {
                flag_e = 1;
                flag_E = 0;
            }
            else if (argv[i][j] == 'E')
            {
                flag_E = 1;
                flag_e = 0;
            }
            else
                goto end_options; // Option non reconnue, fin du traitement des options
        }
        i++;
    }

end_options:
    // Affichage des arguments restants
    for (int j = i; j < argc; j++)
    {
        if (j > i)
            putchar(' ');
        if (flag_e == 1 && flag_E == 0)
        {
            print_with_escape(argv[j]);
        }
        else
        {
            fputs(argv[j], stdout);
        }
    }

    // Ajouter un saut de ligne si -n n'est pas activé
    if (!flag_n)
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
    return 0; // Jamais atteint mais pour eviter des warnings.
}
