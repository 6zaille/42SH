#include "builtins.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static int flag_e = 0;
static int flag_n = 0;
static int i = 1;
static int is_option = 1;



int builtin_echo(int argc, char **argv)
{
    while (i < argc && argv[i][0] == '-' && argv[i][1] != '\0')
    {
        for (size_t j = 1; argv[i][j]; j++)
        {
            if (argv[i][j] == 'e')
            {
                flag_e = 0;
            }
            else if (argv[i][j] == 'E')
            {
                flag_e = 1;
            }
            else if (argv[i][j] == 'n')
            {
                flag_n = 1;
            }
            else
            {
                is_option =
                    0; // Non reconnu, sortir et considérer comme argument
                break;
            }
        }
        if (!is_option)
            break; // Traiter comme argument normal
        i++;
    }

    // Affichage des arguments restants
    for (int j = i; j < argc; j++)
    {
        if (j > i)
        {
            printf(" ");
        }
        if (flag_e == 1)
        {
            for (char *p = argv[j]; *p; p++)
            {
                if (*p == '\\')
                {
                    p++;
                    switch (*p)
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
                        putchar(*p);
                        break;
                    }
                }
                else
                {
                    putchar(*p);
                }
            }
        }
        else
        {
            // Affichage brut (mode -E ou par défaut)
            printf("%s", argv[j]);
        }
    }

    // Ajouter un saut de ligne si -n n'est pas activé
    if (!flag_n)
    {
        printf("\n");
    }
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