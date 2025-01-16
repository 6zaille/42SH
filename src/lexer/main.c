#include <stdio.h>
#include <stdlib.h>

#include "lexer.h"
#include "token.h"

void print_token(struct token tok)
{
    const char *type_str;
    switch (tok.type)
    {
    case TOKEN_IF:
        type_str = "TOKEN_IF";
        break;
    case TOKEN_THEN:
        type_str = "TOKEN_THEN";
        break;
    case TOKEN_ELIF:
        type_str = "TOKEN_ELIF";
        break;
    case TOKEN_ELSE:
        type_str = "TOKEN_ELSE";
        break;
    case TOKEN_FI:
        type_str = "TOKEN_FI";
        break;
    case TOKEN_SEMICOLON:
        type_str = "TOKEN_SEMICOLON";
        break;
    case TOKEN_NEWLINE:
        type_str = "TOKEN_NEWLINE";
        break;
    case TOKEN_WORD:
        type_str = "TOKEN_WORD";
        break;
    case TOKEN_SINGLE_QUOTE:
        type_str = "TOKEN_SINGLE_QUOTE";
        break;
    case TOKEN_EOF:
        type_str = "TOKEN_EOF";
        break;
    case TOKEN_ERROR:
        type_str = "TOKEN_ERROR";
        break;
    default:
        type_str = "UNKNOWN";
        break;
    }

    printf("Token: { type: %s, value: %s }\n", type_str,
           tok.value ? tok.value : "NULL");
}

int main()
{
    char input[1024];

    printf("Entrez une chaîne à analyser : ");
    if (!fgets(input, sizeof(input), stdin))
    {
        perror("fgets");
        return EXIT_FAILURE;
    }

    struct lexer *lexer = lexer_init(input);
    if (!lexer)
    {
        fprintf(stderr, "Erreur lors de l'initialisation du lexer.\n");
        return EXIT_FAILURE;
    }

    struct token tok;
    do
    {
        tok = lexer_next_token(lexer);
        print_token(tok);
        free(tok.value);
    } while (tok.type != TOKEN_EOF);

    lexer_destroy(lexer);
    return EXIT_SUCCESS;
}