#include "ast.h"

#include "parser.h"

int last_exit_status = 0;

void ast_eval(struct ast *node) {
    if (!node) {
        return;
    }
    switch (node->type) {
    case AST_SIMPLE_COMMAND: {
        struct ast_command_data *data = (struct ast_command_data *)node->data;
        if (!data || !data->args)
            return;

        size_t argc = 0;
        while (data->args[argc] != NULL) {
            argc++;
        }

        last_exit_status = execute_command(argc, data->args);
        break;
    }

    case AST_LIST:
        for (size_t i = 0; i < node->children_count ; i++) {
            ast_eval(node->children[i]);
        }
        break;

    case AST_PIPELINE: {
        int num_commands = node->children_count;
        int **pipes = malloc((num_commands - 1) * sizeof(int *));
        for (int i = 0; i < num_commands - 1; i++) {
            pipes[i] = malloc(2 * sizeof(int));
            if (pipe(pipes[i]) == -1) {
                perror("pipe");
                last_exit_status = 1;
                for (int j = 0; j <= i; j++) {
                    free(pipes[j]);
                }
                free(pipes);
                return;
            }
        }

        for (int i = 0; i < num_commands; i++) {
            pid_t pid = fork();
            if (pid == 0) {
                if (i > 0) {
                    dup2(pipes[i - 1][0], STDIN_FILENO);
                }
                if (i < num_commands - 1) {
                    dup2(pipes[i][1], STDOUT_FILENO);
                }
                for (int j = 0; j < num_commands - 1; j++) {
                    close(pipes[j][0]);
                    close(pipes[j][1]);
                }
                ast_eval(node->children[i]);
                exit(last_exit_status);
            }
        }

        for (int i = 0; i < num_commands - 1; i++) {
            close(pipes[i][0]);
            close(pipes[i][1]);
            free(pipes[i]);
        }
        free(pipes);

        for (int i = 0; i < num_commands; i++) {
            int status;
            wait(&status);
            if (WIFEXITED(status)) {
                last_exit_status = WEXITSTATUS(status);
            }
        }
        break;
    }

    case AST_IF: {
        struct ast_if_data *data = (struct ast_if_data *)node->data;
        if (!data)
            return;

        int condition_status = 1;
        if (data->condition && data->condition->children_count > 0) {
            for (size_t i = 0; i < data->condition->children_count; i++) {
                struct ast *child = data->condition->children[i];
                ast_eval(child);
                condition_status = last_exit_status;
            }
        }

        if (condition_status == 0 && data->then_branch) {
            ast_eval(data->then_branch);
        } else if (data->else_branch) {
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
    case AST_NEGATION: {
        struct ast *child = node->children[0];
        ast_eval(child);
        last_exit_status = (last_exit_status == 0) ? 1 : 0;
        break;
    }
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
