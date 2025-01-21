#define _POSIX_C_SOURCE 200809L
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "execution/builtins.h"
#include "execution/exec.h"
#include "lexer/lexer.h"
#include "lexer/token.h"
#include "parser/ast.h"
#include "parser/parser.h"
#include "utils/utils.h"
/*
const char *token_type_to_string(enum token_type type)
{
    switch (type)
    {
    case TOKEN_WORD:
        return "WORD";
    case TOKEN_ASSIGNMENT:
        return "ASSIGNMENT";
    case TOKEN_EOF:
        return "EOF";
    case TOKEN_NEWLINE:
        return "NEWLINE";
    case TOKEN_PIPE:
        return "PIPE";
    case TOKEN_SEMICOLON:
        return "SEMICOLON";
    case TOKEN_REDIRECT_IN:
        return "REDIRECT_IN";
    case TOKEN_REDIRECT_OUT:
        return "REDIRECT_OUT";
    case TOKEN_REDIRECT_APPEND:
        return "REDIRECT_APPEND";
    case TOKEN_REDIRECT_DUP_OUT:
        return "REDIRECT_DUP_OUT";
    case TOKEN_REDIRECT_DUP_IN:
        return "REDIRECT_DUP_IN";
    case TOKEN_REDIRECT_CLOBBER:
        return "REDIRECT_CLOBBER";
    case TOKEN_REDIRECT_RW:
        return "REDIRECT_RW";
    case TOKEN_NEGATION:
        return "NEGATION";
    case TOKEN_IF:
        return "IF";
    case TOKEN_THEN:
        return "THEN";
    case TOKEN_ELIF:
        return "ELIF";
    case TOKEN_ELSE:
        return "ELSE";
    case TOKEN_FI:
        return "FI";
    case TOKEN_VARIABLE:
        return "VARIABLE";
    default:
        return "UNKNOWN";
    }
}*/

void print_tokens(struct lexer *lexer)
{
    struct token *token = NULL;
    while ((token = lexer_next_token(lexer)) != NULL
           && token->type != TOKEN_EOF)
    {
        //printf("Token: Type=%s, Value=%s\n", token_type_to_string(token->type),
        //       token->value);
        token_free(token);
    }
    //printf("Token: Type=%s, Value=%s\n", token_type_to_string(token->type),
    //       token->value);
}

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

int handle_command_line_argument(int argc, char **argv, char **buffer)
{
    if (argc < 3)
    {
        fprintf(stderr, "Error: Missing argument for -c\n");
        fprintf(stderr, "Usage: 42sh [-c COMMAND] [SCRIPT] [OPTIONS...]\n");
        return 127;
    }
    *buffer = strdup(argv[2]);
    if (!*buffer)
    {
        perror("Memory allocation error");
        return 2;
    }
    return 0;
}

int handle_file_input(const char *filename, char **buffer)
{
    FILE *input_file = fopen(filename, "r");
    if (!input_file)
    {
        perror("Error opening file");
        return 127;
    }

    fseek(input_file, 0, SEEK_END);
    size_t buffer_size = ftell(input_file);
    fseek(input_file, 0, SEEK_SET);

    *buffer = malloc(buffer_size + 1);
    if (!*buffer)
    {
        perror("Memory allocation error");
        fclose(input_file);
        return 2;
    }

    size_t read_size = fread(*buffer, 1, buffer_size, input_file);
    if (read_size != buffer_size)
    {
        fprintf(stderr,
                "Error reading the file. Expected %zu bytes, got %zu bytes.\n",
                buffer_size, read_size);
        free(*buffer);
        fclose(input_file);
        return 2;
    }
    (*buffer)[buffer_size] = '\0';

    fclose(input_file);
    return 0;
}

int main(int argc, char **argv)
{
    char *buffer = NULL;
    int result = 0;
    init_shell();

    if (argc > 1 && strcmp(argv[1], "-c") == 0)
    {
        result = handle_command_line_argument(argc, argv, &buffer);
    }
    else if (argc > 1)
    {
        result = handle_file_input(argv[1], &buffer);
    }
    else
    {
        fprintf(stderr, "Error: No input provided\n");
        return 127;
    }

    if (result != 0)
    {
        return result;
    }

    struct lexer *lexer = lexer_init(buffer);
    if (!lexer)
    {
        fprintf(stderr, "Failed to initialize lexer\n");
        free(buffer);
        return 2;
    }
    struct token tok = lexer_peek(lexer);
        if (tok.type == TOKEN_NEWLINE)
        {
            fprintf(stderr, "command not found\n");
            return 127;
        }
    int flag = 0;
    while (1)
    {
        tok = lexer_peek(lexer);
        
        if (tok.type == TOKEN_EOF)
            break;
        if (flag == 1)
        {
            lexer_pop(lexer);
        }
        struct ast *ast = parser_parse(lexer);
        if (!ast)
        {
            fprintf(stderr, "ast pas créé\n");
            break;
        }
        flag = 1;
        ast_eval(ast);
        ast_free(ast);
    }
    lexer_destroy(lexer);
    free(buffer);
    free(pwd);
    free(oldpwd);
    return last_exit_status;
}