#include <stdio.h>
#include <stdlib.h>

#include "../lexer/lexer.h"
#include "../lexer/token.h"
#include "ast.h"
#include "parser.h"


// Fonction pour afficher l'AST pour le debug
void pretty_print_ast(struct ast *node, int depth)
{
    if (!node)
        return;

    for (int i = 0; i < depth; i++)
        printf("  ");

    printf("Node (children: %zu): Token: %s, Value: %zd\n",
           node->children_count, node->token.value ? node->token.value : "NULL",
           node->value);

    for (size_t i = 0; i < node->children_count; i++)
    {
        pretty_print_ast(node->children[i], depth + 1);
    }
}

int main(int argc, char **argv)
{
    
    if (argc != 2)
    {
        fprintf(stderr, "Usage: %s \"command\"\n", argv[0]);
        return EXIT_FAILURE;
    }

    printf("[DEBUG] Initializing lexer...\n");
    struct lexer *lexer = lexer_init(argv[1]);
    if (!lexer)
    {
        fprintf(stderr, "[ERROR] Failed to initialize lexer.\n");
        return EXIT_FAILURE;
    }

    printf("[DEBUG] Starting parser...\n");
    enum parser_status status;
    struct ast *root = parser_parse(lexer, &status);

    if (status != PARSER_OK)
    {
        fprintf(stderr, "[ERROR] Parsing failed with status: %d.\n", status);
        lexer_destroy(lexer);
        return EXIT_FAILURE;
    }

    printf("[DEBUG] Parsing succeeded. Printing AST:\n");
    pretty_print_ast(root, 0);

    // Libérer la mémoire
    ast_free(root);
    lexer_destroy(lexer);
    return EXIT_SUCCESS;
}
