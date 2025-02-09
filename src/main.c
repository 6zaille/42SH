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

int args_count = 0;

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
/*
void print_tokens(struct lexer *lexer)
{
    struct token *token = NULL;
    while ((token = lexer_next_token(lexer)) != NULL
           && token->type != TOKEN_EOF)
    {
        // printf("Token: Type=%s, Value=%s\n",
        // token_type_to_string(token->type),
        //        token->value);
        token_free(token);
    }
    // printf("Token: Type=%s, Value=%s\n", token_type_to_string(token->type),
    //        token->value);
}
*/ /*
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
} */

int handle_command_line_argument(int argc, char **argv, char **buffer)
{
    if (argc < 3)
    {
        fprintf(stderr, "Error: Missing argument for -c\n");
        fprintf(stderr, "Usage: 42sh [-c COMMAND] [SCRIPT] [OPTIONS...]\n");
        return 127;
    }
    *buffer = strdup(argv[2]);
    /*if (!*buffer)
    {
        perror("Memory allocation error");
        return 2;
    }*/
    return 0;
}

static int handle_file_input(const char *filename, char **buffer)
{
    FILE *input_file = fopen(filename, "r");
    /*if (!input_file)
    {
        perror("Error opening file");
        return 127;
    }*/

    fseek(input_file, 0, SEEK_END);
    size_t buffer_size = ftell(input_file);
    fseek(input_file, 0, SEEK_SET);

    *buffer = malloc(buffer_size + 1);
    /*if (!*buffer)
    {
        perror("Memory allocation error");
        fclose(input_file);
        return 2;
    }*/

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

void init_args(int argc, char **argv)
{
    args_count = argc - 2;
    for (size_t i = 2; i < (size_t)argc; i++)
    {
        args[i - 2] = argv[i];
    }
}

void init_variables(int argc, char **argv)
{
    set_variable("0", argv[1]);
    for (size_t i = 1; i < (size_t)argc - 1; i++)
    {
        char var_name[30];
        snprintf(var_name, 30, "%zu", i);
        set_variable(var_name, argv[i + 1]);
    }
}

static int handle_stdin_mode(void)
{
    char *accumulated_input = calloc(1, sizeof(char));
    size_t buffer_size = 1024;
    char *buffer = malloc(buffer_size);
    while (1)
    {
        ssize_t bytes_read = read(STDIN_FILENO, buffer, buffer_size - 1);
        if (bytes_read == -1)
        {
            perror("read");
            free(buffer);
            free(accumulated_input);
            return 1;
        }

        if (bytes_read == 0)
            break;

        buffer[bytes_read] = '\0';

        size_t new_length = strlen(accumulated_input) + bytes_read + 1;
        char *new_accumulated = realloc(accumulated_input, new_length);

        accumulated_input = new_accumulated;
        strcat(accumulated_input, buffer);

        struct lexer *lexer = lexer_init(accumulated_input);
        if (!lexer)
        {
            fprintf(stderr, "Erreur d'initialisation du lexer\n");
            free(buffer);
            free(accumulated_input);
            return 1;
        }

        while (1)
        {
            struct token tok = lexer_peek(lexer);
            if (tok.type == TOKEN_EOF)
                break;

            struct ast *ast = parser_parse(lexer);
            if (ast)
            {
                ast_eval(ast);
                ast_free(ast);
            }
            else
            {
                tok = lexer_peek(lexer);
                if (tok.type == TOKEN_EOF)
                {
                    break;
                }
            }
        }

        lexer_destroy(lexer);
    }

    free(buffer);
    free(accumulated_input);

    return 0;
}
void process_command_line_argument(int argc, char **argv, char **buffer,
                                   int *result)
{
    init_shell();
    *result = handle_command_line_argument(argc, argv, buffer);
}

void process_file_input(int argc, char **argv, char **buffer, int *result)
{
    init_shell();
    init_args(argc, argv);
    init_variables(argc, argv);
    *result = handle_file_input(argv[1], buffer);
}

int process_tokens(struct lexer *lexer)
{
    struct token tok = lexer_peek(lexer);
    while (tok.type == TOKEN_NEWLINE || tok.type == TOKEN_EOF)
    {
        if (tok.type == TOKEN_EOF)
            return 0;
        lexer_pop(lexer);
        tok = lexer_peek(lexer);
    }
    while (1)
    {
        tok = lexer_peek(lexer);

        if (tok.type == TOKEN_EOF)
            break;
        struct ast *ast = parser_parse(lexer);
        if (!ast)
        {
            if (status_error != 0)
            {
                fprintf(stderr, "ast pas créé\n");
                return status_error;
            }
            break;
        }

        ast_eval(ast);
        ast_free(ast);
    }
    return 0;
}

int main(int argc, char **argv)
{
    char *buffer = NULL;
    int result = 0;
    if (argc == 1)
    {
        return handle_stdin_mode();
    }
    if (argc > 1 && strcmp(argv[1], "-c") == 0)
    {
        process_command_line_argument(argc, argv, &buffer, &result);
    }
    else if (argc > 1 && strstr(argv[1], ".sh") != NULL)
    {
        process_file_input(argc, argv, &buffer, &result);
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
    /*if (!lexer)
    {
        fprintf(stderr, "lexer not initialized\n");
        free(buffer);
        return 2;
    }*/

    int check = process_tokens(lexer);
    if (check != 0)
    {
        return check;
    }

    lexer_destroy(lexer);
    free(buffer);
    free(pwd);
    free(oldpwd);
    return last_exit_status;
}
