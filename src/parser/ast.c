#include "ast.h"

#include "parser.h"

int last_exit_status = 0;

void ast_eval(struct ast *node)
{
    // ast_pretty_print(node, 0);
    if (!node)
    {
        return;
    }
    switch (node->type)
    {
    case AST_SIMPLE_COMMAND: {
        struct ast_command_data *data = (struct ast_command_data *)node->data;
        if (!data || !data->args)
            return;

        pid_t pid = fork();
        if (pid == 0)
        {
            execvp(data->args[0], data->args);
            perror("execvp");
            exit(EXIT_FAILURE);
        }
        else if (pid > 0)
        {
            int status;
            waitpid(pid, &status, 0);

            if (WIFEXITED(status))
            {
                last_exit_status = WEXITSTATUS(status);
            }
            else
            {
                last_exit_status = 1;
            }
        }
        else
        {
            perror("fork");
            last_exit_status = 1;
        }
        break;
    }

    case AST_LIST:
        for (size_t i = 0; i < node->children_count; i++)
        {
            ast_eval(node->children[i]);
        }
        break;

    case AST_IF: {
        struct ast_if_data *data = (struct ast_if_data *)node->data;
        if (!data)
            return;

        int condition_status = 1;
        if (data->condition && data->condition->children_count > 0)
        {
            for (size_t i = 0; i < data->condition->children_count; i++)
            {
                struct ast *child = data->condition->children[i];
                ast_eval(child);
                condition_status = last_exit_status;
            }
        }

        if (condition_status == 0 && data->then_branch)
        {
            ast_eval(data->then_branch);
        }
        else if (data->else_branch)
        {
            ast_eval(data->else_branch);
        }
        break;
    }

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
        printf(i == depth - 1 ? "|---" : "|   ");

    switch (node->type)
    {
    case AST_LIST:
        printf("LIST\n");
        break;
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
    case AST_IF:
        printf("IF\n");
        printf("|   Condition:\n");
        ast_pretty_print(((struct ast_if_data *)node->data)->condition,
                         depth + 1);
        printf("|   Then:\n");
        ast_pretty_print(((struct ast_if_data *)node->data)->then_branch,
                         depth + 1);
        if (((struct ast_if_data *)node->data)->else_branch)
        {
            printf("|   Else:\n");
            ast_pretty_print(((struct ast_if_data *)node->data)->else_branch,
                             depth + 1);
        }
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
