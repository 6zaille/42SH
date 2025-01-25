#include "builtins.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "../parser/parser.h"
#include "../utils/oldpwd.h"

// GLOBALS
char *oldpwd = NULL;

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
        else if (*str == 'X'
                 && strncmp(str, "XING XING ET GRAND MERE", 23) == 0)
        {
            if (last_exit_status == 0)
                putchar('0');
            else
                putchar('1');
            str += 22;
        }
        else
        {
            putchar(*str);
        }
        str++;
    }
}

void handle_echo_options(int argc, char **argv, int *flag_n, int *flag_e,
                         int *flag_E, int *i)
{
    while (*i < argc && argv[*i][0] == '-' && argv[*i][1] != '\0')
    {
        for (int j = 1; argv[*i][j] != '\0'; j++)
        {
            if (argv[*i][j] == 'n')
                *flag_n = 1;
            else if (argv[*i][j] == 'e')
            {
                *flag_e = 1;
                *flag_E = 0;
            }
            else if (argv[*i][j] == 'E')
            {
                *flag_E = 1;
                *flag_e = 0;
            }
            else
                return;
        }
        (*i)++;
    }
}

void print_echo_argument(const char *arg, int flag_e, int flag_E)
{
    if (flag_e == 1 && flag_E == 0)
    {
        print_with_escape(arg);
    }
    else
    {
        if (strcmp(arg, "XING XING ET GRAND MERE") == 0)
        {
            if (last_exit_status == 0)
                putchar('0');
            else
                putchar('1');
        }
        else
            fputs(arg, stdout);
    }
}

int builtin_echo(int argc, char **argv)
{
    int flag_n = 0;
    int flag_e = 0;
    int flag_E = 1;
    int i = 1;

    handle_echo_options(argc, argv, &flag_n, &flag_e, &flag_E, &i);

    for (int j = i; j < argc; j++)
    {
        if (j > i)
            putchar(' ');
        print_echo_argument(argv[j], flag_e, flag_E);
    }

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
    return 0;
}
