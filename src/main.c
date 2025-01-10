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
    FILE *input_file = stdin; // Par défaut, lire depuis stdin
    int pretty_print = 0;

    if (argc > 1 && strcmp(argv[1], "-c") == 0)
    {
        if (argc < 3)
        {
            fprintf(stderr, "Error: Missing argument for -c\n");
            fprintf(stderr, "Usage: 42sh [-c COMMAND] [SCRIPT] [OPTIONS...]\n");
            return 2;
        }
        char *command = argv[2];
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
        return 0;
    }
    else if (argc > 1)
    {
        input_file = fopen(argv[1], "r");
        if (!input_file)
        {
            perror("Error opening file");
            return 2;
        }
    }

    // Lecture depuis le fichier (ou stdin si aucun fichier n'est passé)
    char *line = NULL;
    size_t len = 0;

    while (custom_getline(&line, &len, input_file) != -1)
    {
        struct lexer *lexer = lexer_init(line);
        if (!lexer)
        {
            fprintf(stderr, "Failed to initialize lexer\n");
            free(line);
            if (input_file != stdin)
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
            if (input_file != stdin)
                fclose(input_file);
            return 2;
        }

        eval_ast(ast);
        ast_free(ast);
    }

    free(line);
    if (input_file != stdin)
        fclose(input_file);

    return 0;
}