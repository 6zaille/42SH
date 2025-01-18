#include <stdio.h>
#include <stdlib.h>

#include "lexer.h"
#include "token.h"
const char *token_type_to_string(enum token_type type)
{
    switch (type)
    {
    case TOKEN_IF:
        return "TOKEN_IF";
    case TOKEN_THEN:
        return "TOKEN_THEN";
    case TOKEN_ELIF:
        return "TOKEN_ELIF";
    case TOKEN_ELSE:
        return "TOKEN_ELSE";
    case TOKEN_FI:
        return "TOKEN_FI";
    case TOKEN_SEMICOLON:
        return "TOKEN_SEMICOLON";
    case TOKEN_NEWLINE:
        return "TOKEN_NEWLINE";
    case TOKEN_WORD:
        return "TOKEN_WORD";
    case TOKEN_SINGLE_QUOTE:
        return "TOKEN_SINGLE_QUOTE";
    case TOKEN_EOF:
        return "TOKEN_EOF";
    case TOKEN_ERROR:
        return "TOKEN_ERROR";
    case TOKEN_PIPE:
        return "TOKEN_PIPE";
    case TOKEN_NEGATION:
        return "TOKEN_NEGATION";
    default:
        return "UNKNOWN";
    }
}

void print_token(struct token *tok)
{
    const char *type_str = token_type_to_string(tok->type);
    printf("Token: { type: %s, value: %s }\n", type_str,
           tok->value ? tok->value : "NULL");
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

    struct token *tok;
    do
    {
        tok = lexer_next_token(lexer);
        print_token(tok);
        free(tok->value);
    } while (tok->type != TOKEN_EOF);

    lexer_destroy(lexer);
    return EXIT_SUCCESS;
}
