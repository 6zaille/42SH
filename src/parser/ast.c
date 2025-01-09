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

// Function to traverse a branch downwards
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


// Function to evaluate an AST
void __eval_ast(struct ast *root)
{
    if (!root)
        return;
    // Process the current node after its children
    if (root->token.value)
    {
        // Allocate memory for the arguments array
        int taille = traverse_branch(root);
        char **args = malloc((taille) * sizeof(char *));
        if (!args)
            return;

        // Populate the arguments array
        args[0] = root->token.value;
        int save = 1;

        struct ast *current = root;
        while (current->children_count > 0)
        {
            current = current->children[0];
            args[save] = current->token.value;
            save++;
        }

        // Execute the command with the arguments
        for (size_t k = 0; k < taille; k++)
        {
            printf("args[%zu] = %s\n", k, args[k]);
        }
        int res = execute_command(taille, args);

        // Free the arguments array
        free(args);
    }
}

void eval_ast(struct ast *root)
{
    for (size_t i = 0; i < root->children_count; i++)
    {
        __eval_ast(root->children[i]);
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