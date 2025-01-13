#include "ast.h"

#include "parser.h"

void ast_eval(struct ast *node)
{
    //ast_pretty_print(node, 0);
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
            /*
            int argc = 1;
            while (data->args[argc] != NULL)
            {
                argc++;
            }
            printf("argc = %d\n", argc);
            execute_command(argc, data->args);*/
            execvp(data->args[0], data->args);
            perror("execvp");
            exit(EXIT_FAILURE);
        }
        else if (pid > 0)
        {
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

    case AST_IF: {
        struct ast_if_data *data = (struct ast_if_data *)node->data;
        if (!data)
            return;

        int condition_status = 1;
        if (data->condition) {
            struct ast *current = data->condition;

            while (current) {
                ast_eval(current);
                condition_status = WEXITSTATUS(0);

                if (current->children_count > 0)
                    current = current->children[current->children_count - 1];
                else
                    current = NULL;
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
    case AST_SIMPLE_COMMAND: {
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
    }

    case AST_LIST:
        printf("LIST\n");
        break;

    case AST_IF: {
        printf("IF\n");
        struct ast_if_data *data = (struct ast_if_data *)node->data;
        if (data)
        {
            printf("%*sCondition:\n", depth * 2, "");
            ast_pretty_print(data->condition, depth + 1);
            printf("%*sThen:\n", depth * 2, "");
            ast_pretty_print(data->then_branch, depth + 1);
            if (data->else_branch)
            {
                printf("%*sElse:\n", depth * 2, "");
                ast_pretty_print(data->else_branch, depth + 1);
            }
        }
        break;
    }

    default:
        printf("UNKNOWN\n");
    }

    // Parcours des enfants dans la structure de l'arbre
    for (size_t i = 0; i < node->children_count; i++)
    {
        ast_pretty_print(node->children[i], depth + 1);
    }
}
