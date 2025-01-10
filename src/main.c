#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "execution/builtins.h"
#include "execution/exec.h"
#include "lexer/lexer.h"
#include "parser/ast.h"
#include "parser/parser.h"
#include "utils/utils.h"

// Implémentation personnalisée de getline
ssize_t custom_getline(char **lineptr, size_t *n, FILE *stream)
{
    if (!lineptr || !n || !stream)
    {
        return -1;
    }

    if (*lineptr == NULL || *n == 0)
    {
        *n = 128; // Taille initiale par défaut
        *lineptr = malloc(*n);
        if (*lineptr == NULL)
        {
            return -1;
        }
    }

    size_t pos = 0;
    int c;

    while ((c = fgetc(stream)) != EOF)
    {
        if (pos + 1 >= *n)
        {
            *n *= 2; // Augmenter la taille de la ligne
            char *new_ptr = realloc(*lineptr, *n);
            if (!new_ptr)
            {
                return -1;
            }
            *lineptr = new_ptr;
        }

        (*lineptr)[pos++] = c;

        if (c == '\n')
        {
            break;
        }
    }

    if (pos == 0 && c == EOF)
    {
        return -1;
    }

    (*lineptr)[pos] = '\0';
    return (ssize_t)pos;
}

int main(int argc, char **argv)
{
    int pretty_print = 0;
    char *command = NULL;
    FILE *input_file = NULL;

    if (argc == 1)
    {
        return 0;
    }

    for (int i = 1; i < argc; i++)
    {
        if (strcmp(argv[i], "-c") == 0)
        {
            if (i + 1 >= argc)
            {
                fprintf(stderr, "Error: Missing argument for -c\n");
                fprintf(stderr,
                        "Usage: 42sh [-c COMMAND] [FILE] [ARGUMENTS...]\n");
                return 2;
            }
            command = argv[++i];
        }
        else if (strcmp(argv[i], "--pretty-print") == 0)
        {
            pretty_print = 1;
        }
        else if (!input_file && command == NULL)
        {
            command = argv[i];
        }
        else
        {
            fprintf(stderr, "Error: Unexpected argument %s\n", argv[i]);
            fprintf(stderr, "Usage: 42sh [-c COMMAND] [FILE] [ARGUMENTS...]\n");
            return 2;
        }
    }

    if (command)
    {
        struct lexer *lexer = lexer_init(command);
        if (!lexer)
        {
            fprintf(stderr, "Failed to initialize lexer\n");
            return 2;
        }

        enum parser_status status;
        struct ast *ast = parser_parse(lexer, &status);
        lexer_destroy(lexer);

        if (status != PARSER_OK)
        {
            fprintf(stderr, "Syntax error\n");
            return 2;
        }

        if (pretty_print)
        {
            print_arbre(ast, 0);
        }
        else
        {
            eval_ast(ast);
        }

        ast_free(ast);
    }
    else if (input_file)
    {
        char *line = NULL;
        size_t len = 0;
        if (custom_getline(&line, &len, input_file) != -1)
        {
            struct lexer *lexer = lexer_init(line);
            if (!lexer)
            {
                fprintf(stderr, "Failed to initialize lexer\n");
                free(line);
                fclose(input_file);
                return 2;
            }

            enum parser_status status;
            struct ast *ast = parser_parse(lexer, &status);
            lexer_destroy(lexer);

            if (status != PARSER_OK)
            {
                fprintf(stderr, "Syntax error\n");
                free(line);
                fclose(input_file);
                return 2;
            }

            if (pretty_print)
            {
                print_arbre(ast, 0);
            }
            else
            {
                eval_ast(ast);
            }

            ast_free(ast);
        }
        free(line);
        fclose(input_file);
    }

    return 0;
}