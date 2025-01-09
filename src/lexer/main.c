#include <stdio.h>^M$
#include <stdlib.h>^M$
#include "lexer.h"^M$
#include "token.h"^M$
^M$
void print_token(struct token *tok)^M$
{^M$
    if (!tok)^M$
    {^M$
        printf("Error: NULL token\n");^M$
        return;^M$
    }^M$
^M$
    const char *token_names[] = {^M$
        "TOKEN_IF", "TOKEN_THEN", "TOKEN_ELIF", "TOKEN_ELSE", "TOKEN_FI",^M$
        "TOKEN_SEMICOLON", "TOKEN_NEWLINE", "TOKEN_WORD", "TOKEN_SINGLE_QUOTE",^M$
        "TOKEN_EOF", "TOKEN_ERROR"};^M$
^M$
    printf("Token: %s", token_names[tok->type]);^M$
    if (tok->value)^M$
        printf(", Value: '%s'", tok->value);^M$
    printf("\n");^M$
}^M$
^M$
int main(int argc, char **argv)^M$
{^M$
    if (argc != 2)^M$
    {^M$
        fprintf(stderr, "Usage: %s <input_string>\n", argv[0]);^M$
        return EXIT_FAILURE;^M$
    }^M$
^M$
    struct lexer *lexer = lexer_init(argv[1]);^M$
    if (!lexer)^M$
    {^M$
        fprintf(stderr, "Failed to initialize lexer\n");^M$
        return EXIT_FAILURE;^M$
    }^M$
^M$
    struct token *tok;^M$
    while ((tok = lexer_next_token(lexer))->type != TOKEN_EOF)^M$
    {^M$
        print_token(tok);^M$
        token_free(tok);^M$
    }^M$
^M$
    print_token(tok);^M$
    token_free(tok);^M$
    lexer_destroy(lexer);^M$
^M$
    return EXIT_SUCCESS;^M$
}^M$
