#include <stdio.h>
#include <stdlib.h>
#include "parser.h"
#include "../lexer/lexer.h"
#include "ast.h"

int main()
{
    //printf("Début du programme\n");

    const char *input = "echo toto";
    printf("Input: %s\n", input);

    struct lexer *lexer = lexer_init(input);
    if (!lexer)
    {
        fprintf(stderr, "Erreur : échec de l'initialisation du lexer\n");
        return EXIT_FAILURE;
    }
    //printf("Lexer initialisé avec succès\n");

    enum parser_status status;
    struct ast *root = parser_parse(lexer, &status);
    //printf("Parser appelé\n");

    if (status != PARSER_OK)
    {
        fprintf(stderr, "Erreur : parsing échoué\n");
        lexer_destroy(lexer);
        return EXIT_FAILURE;
    }
    //printf("Parsing réussi\n");

    //printf("Parsing réussi ! Voici l'AST :\n");
    print_arbre(root, 0); // Assurez-vous que `print_arbre` est défini dans `ast.c`

    ast_free(root);
    //printf("AST libéré\n");

    lexer_destroy(lexer);
    //printf("Lexer détruit\n");

    //printf("Fin du programme\n");
    return EXIT_SUCCESS;
}
