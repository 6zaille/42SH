#include <stdlib.h>
#include "ast.h"

struct ast *ast_new(enum ast_type type)
{
    struct ast *node = malloc(sizeof(struct ast));
    if (!node)
        return NULL;

    node->type = type;
    node->value = 0;
    node->left = NULL;
    node->right = NULL;
    return node;
}

void ast_free(struct ast *node)
{
    if (!node)
        return;

    ast_free(node->left);
    ast_free(node->right);
    free(node);
}
