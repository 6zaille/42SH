#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "execution/builtins.h"
#include "execution/exec.h"
#include "lexer/lexer.h"
#include "parser/ast.h"
#include "parser/parser.h"
#include "utils/utils.h"

ssize_t custom_getline(char **lineptr, size_t *n, FILE *stream)
{
    if (!lineptr || !n || !stream)
    {
        return -1;
    }

    if (*lineptr == NULL || *n == 0)
    {
        *n = 128;
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
            *n *= 2;
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
    FILE *input_file = stdin;
    int pretty_print = 0;

    if (argc > 1 && strcmp(argv[1], "-c") == 0)
    {
        if (argc < 3)
        {
            fprintf(stderr, "Error: Missing argument for -c\n");
            fprintf(stderr, "Usage: 42sh [-c COMMAND] [SCRIPT] [OPTIONS...]\n");
            return 127;
        }
        char *command = argv[2];
        struct lexer *lexer = lexer_init(command);
        if (!lexer)
        {
            fprintf(stderr, "Failed to initialize lexer\n");
            return 2;
        }

        struct ast *ast = parser_parse(lexer);
        lexer_destroy(lexer);

        if (pretty_print)
        {
            ast_pretty_print(ast, 0);
        }
        else
        {
            ast_eval(ast);
        }

        ast_free(ast);
        exit(0);
    }
    else if (argc > 1)
    {
        input_file = fopen(argv[1], "r");
        if (!input_file)
        {
            perror("Error opening file");
            return 127;
        }
    }
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

        struct ast *ast = parser_parse(lexer);
        lexer_destroy(lexer);

        ast_eval(ast);
        ast_free(ast);
    }

    free(line);
    if (input_file != stdin)
        fclose(input_file);

    exit(0);
}
