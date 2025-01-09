#include "ast.h"

// Function to create a new AST node
struct ast *ast_new(void)
{
    // Allocate memory for the new AST node
    struct ast *node = malloc(sizeof(struct ast));
    if (!node)
    {
        // Print error message if memory allocation fails
        fprintf(stderr, "Failed to allocate memory for new AST node\n");
        return NULL;
    }

    // Initialize the node's fields
    node->token = (struct token){.value = NULL, .type = 0};
    node->value = 0;
    node->children = NULL;
    node->children_count = 0;

    return node;
}

// Function to free an AST node
void ast_free(struct ast *node)
{
    if (!node)
        return;

    // Recursively free all children nodes
    for (size_t i = 0; i < node->children_count; i++)
    {
        ast_free(node->children[i]);
    }
    // Free the children array and the node itself
    free(node->children);
    free(node);
}

// Function to evaluate an AST
void eval_ast(struct ast *root)
{
    if (!root || !root->children)
        return;

    // Iterate over all children of the root node
    for (size_t i = 0; i < root->children_count; i++)
    {
        struct ast *child = root->children[i];
        if (!child || !child->children)
            continue;

        // Allocate memory for the arguments array
        char **args = malloc((child->children_count + 1) * sizeof(char *));
        if (!args)
            return;

        // Populate the arguments array
        args[0] = child->token.value;
        for (size_t j = 0; j < child->children_count; j++)
        {
            args[j+1] = child->children[j]->token.value;
        }

        // Execute the command with the arguments
        int res = execute_command(child->children_count +1 , args);

        // Free the arguments array
        free(args);
    }
}

// Function to print the AST
void print_arbre(struct ast *node)
{
    if (!node)
        return;

    // Print the current node's value and token type
    printf("Node value: %s, Token type: %d\n", node->token.value, node->token.type);

    // Recursively print all children nodes
    for (size_t i = 0; i < node->children_count; i++)
    {
        printf("Child %zu of node value %s, Token type: %d:\n", i, node->token.value, node->token.type);
        print_arbre(node->children[i]);
    }
}
