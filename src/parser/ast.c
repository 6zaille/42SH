#include "ast.h"

struct ast *ast_new(void)
{
    struct ast *node = malloc(sizeof(struct ast));
    if (!node)
    {
        fprintf(stderr, "Failed to allocate memory for new AST node\n");
        return NULL;
    }

    node->token = (struct token){ .value = NULL, .type = 0 };
    node->value = 0;
    node->children = NULL;
    node->children_count = 0;

    return node;
}

void ast_free(struct ast *node)
{
    if (!node)
        return;

    for (size_t i = 0; i < node->children_count; i++)
    {
        ast_free(node->children[i]);
    }
    free(node->children);
    free(node);
}

int traverse_branch(struct ast *node)
{
    if (!node)
        return 0;

    int max_depth = 0;

    for (size_t i = 0; i < node->children_count; i++)
    {
        int depth = traverse_branch(node->children[i]);
        if (depth > max_depth)
            max_depth = depth;
    }

    return max_depth + 1;
}

void __eval_ast(struct ast *root)
{
    if (!root)
        return;
    if (root->token.value)
    {
        int taille = traverse_branch(root);
        char **args = malloc((taille) * sizeof(char *));
        if (!args)
            return;

        args[0] = root->token.value;
        int save = 1;

        struct ast *current = root;
        while (current->children_count > 0)
        {
            current = current->children[0];
            args[save] = current->token.value;
            save++;
        }
        int res = execute_command(taille, args);
        if (res == 1000000)
        {
            fprintf(stderr, "Erreur : commande inconnue\n");
        }
        free(args);
    }
}

void eval_ast(struct ast *root)
{
    // print_arbre(root, 0);
    for (size_t i = 0; i < root->children_count; i++)
    {
        __eval_ast(root->children[i]);
    }
}

void print_arbre(struct ast *node, int depth)
{
    if (!node)
        return;
    for (int i = 0; i < depth; i++)
    {
        if (i == depth - 1)
            printf("  └─―――");
        else
            printf("  │ ");
        // printf("   ");
    }
    printf("Node value: %s\n", node->token.value);

    for (size_t i = 0; i < node->children_count; i++)
    {
        print_arbre(node->children[i], depth + 1);
    }
}
