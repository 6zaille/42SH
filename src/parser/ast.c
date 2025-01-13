#include "ast.h"

#include "parser.h"

void ast_eval(struct ast *node)
{
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

    for (int i = 0; i < depth; i++)
    {
        printf("  ");
    }

    switch (node->type)
    {
    case AST_SIMPLE_COMMAND:
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
        break;

    case AST_LIST:
        printf("LIST\n");
        break;

    default:
        printf("UNKNOWN\n");
        break;
    }

    for (size_t i = 0; i < node->children_count; i++)
    {
        ast_pretty_print(node->children[i], depth + 1);
    }
}
