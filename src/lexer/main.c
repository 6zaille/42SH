#include <stdio.h>
#include <stdlib.h>
#include "lexer.h"
#include "token.h"

void print_token(struct token *tok)
{
    if (!tok)
    {
        printf("Error: NULL token\n");
        return;
    }

    const char *token_names[] = {
        "TOKEN_IF", "TOKEN_THEN", "TOKEN_ELIF", "TOKEN_ELSE", "TOKEN_FI",
        "TOKEN_SEMICOLON", "TOKEN_NEWLINE", "TOKEN_WORD", "TOKEN_SINGLE_QUOTE",
        "TOKEN_EOF", "TOKEN_ERROR"};

    printf("Token: %s", token_names[tok->type]);
    if (tok->value)
        printf(", Value: '%s'", tok->value);
    printf("\n");
}

int main(int argc, char **argv)
{
    if (argc != 2)
    {
        fprintf(stderr, "Usage: %s <input_string>\n", argv[0]);
        return EXIT_FAILURE;
    }

    struct lexer *lexer = lexer_init(argv[1]);
    if (!lexer)
    {
        fprintf(stderr, "Failed to initialize lexer\n");
        return EXIT_FAILURE;
    }

    struct token *tok;
    while ((tok = lexer_next_token(lexer))->type != TOKEN_EOF)
    {
        print_token(tok);
        token_free(tok);
    }

    print_token(tok); // Print EOF token
    token_free(tok);
    lexer_destroy(lexer);

    return EXIT_SUCCESS;
}
