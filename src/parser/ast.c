#include "ast.h"

#include "parser.h"

void ast_eval(struct ast *node)
{
    // ast_pretty_print(node, 0);
    if (!node)
        return;

    switch (node->type)
    {
    case AST_SIMPLE_COMMAND: {
        struct ast_command_data *data = (struct ast_command_data *)node->data;
        if (!data || !data->args)
            return;

        pid_t pid = fork();
        if (pid == 0)
        { // Processus enfant
            execvp(data->args[0], data->args);
            perror("execvp");
            exit(EXIT_FAILURE);
        }
        else if (pid > 0)
        { // Processus parent
            int status;
            waitpid(pid, &status, 0);
        }
        else
        {
            perror("fork");
        }
        break;
    }

    case AST_LIST:
        for (size_t i = 0; i < node->children_count; i++)
        {
            ast_eval(node->children[i]);
        }
        break;

    default:
        fprintf(stderr, "Unsupported AST node type: %d\n", node->type);
        break;
    }
}

void ast_pretty_print(struct ast *node, int depth)
{
    if (!node)
        return;

    // Affichage de l'indentation et du connecteur visuel
    for (int i = 0; i < depth; i++)
        printf(i == depth - 1 ? "|---" : "|   ");

    // Affichage des informations du nœud
    if (node->type == AST_LIST)
    {
        printf("LIST\n");
    }
    else if (node->type == AST_SIMPLE_COMMAND && depth != 0)
    {
        printf("SIMPLE_COMMAND");
        struct ast_command_data *data = (struct ast_command_data *)node->data;
        if (data && data->args)
        {
            for (size_t i = 0; data->args[i]; i++)
            {
                printf(" %s", data->args[i]);
            }
        }
        printf("\n");
    }
    else if (depth != 0)
    {
        printf("UNKNOWN\n");
    }

    // Parcours des enfants dans la structure de l'arbre
    for (size_t i = 0; i < node->children_count; i++)
    {
        ast_pretty_print(node->children[i], depth + 1);
    }
}
