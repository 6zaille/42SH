#include "ast.h"

#include "parser.h"

int last_exit_status = 0;
int loop_running = 1;

void ast_eval(struct ast *node)
{
    if (!node)
    {
        return;
    }
    switch (node->type)
    {
    case AST_SIMPLE_COMMAND: {
        struct ast_command_data *data = (struct ast_command_data *)node->data;
        if (!data || !data->args || !data->args[0])
        {
            last_exit_status = 1;
            return;
        }
        const char *cmd = data->args[0];

        // Gestion de 'break'
        if (strcmp(cmd, "break") == 0)
        {
            // Met fin à la boucle en définissant une variable globale ou un
            // drapeau
            loop_running =
                0; // Définir `loop_running` comme une variable globale
            last_exit_status = 0; // Indiquer un succès
            return;
        }
        int argc = 0;
        while (data->args[argc] != NULL)
        {
            argc++;
        }

        last_exit_status = execute_command(argc, data->args);
        break;
    }

    case AST_LIST:
        for (size_t i = 0; i < node->children_count; i++)
        {
            ast_eval(node->children[i]);
        }
        break;

    case AST_PIPELINE: {
        int num_commands = node->children_count;
        int **pipes = malloc((num_commands - 1) * sizeof(int *));
        for (int i = 0; i < num_commands - 1; i++)
        {
            pipes[i] = malloc(2 * sizeof(int));
            if (pipe(pipes[i]) == -1)
            {
                perror("pipe");
                last_exit_status = 1;
                for (int j = 0; j <= i; j++)
                {
                    free(pipes[j]);
                }
                free(pipes);
                return;
            }
        }

        for (int i = 0; i < num_commands; i++)
        {
            pid_t pid = fork();
            if (pid == 0)
            {
                if (i > 0)
                {
                    dup2(pipes[i - 1][0], STDIN_FILENO);
                }
                if (i < num_commands - 1)
                {
                    dup2(pipes[i][1], STDOUT_FILENO);
                }
                for (int j = 0; j < num_commands - 1; j++)
                {
                    close(pipes[j][0]);
                    close(pipes[j][1]);
                }
                ast_eval(node->children[i]);
                exit(last_exit_status);
            }
        }

        for (int i = 0; i < num_commands - 1; i++)
        {
            close(pipes[i][0]);
            close(pipes[i][1]);
            free(pipes[i]);
        }
        free(pipes);

        for (int i = 0; i < num_commands; i++)
        {
            int status;
            wait(&status);
            if (WIFEXITED(status))
            {
                last_exit_status = WEXITSTATUS(status);
            }
        }
        break;
    }
    case AST_WHILE: {
        struct ast *condition = node->children[0];
        struct ast *body = node->children[1];

        // printf("Evaluating while loop...\n");
        while (loop_running)
        {
            ast_eval(condition);

            if (last_exit_status != 0)
            {
                // printf("While loop condition failed, exiting loop...\n");
                break;
            }

            ast_eval(body);
        }
        // Réinitialisez la variable après la boucle
        loop_running = 1;
        break;
    }

    case AST_IF: {
        struct ast_if_data *data = (struct ast_if_data *)node->data;
        if (!data)
        {
            // fprintf(stderr, "Ast node if n'existe pas\n");
            return;
        }
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
        else if (!data->then_branch && !data->else_branch)
        {
            fprintf(stderr,
                    "[ERROR] AST_IF missing 'then' or 'else' branch.\n");
        }
        break;
    }
    case AST_NEGATION: {
        if (node->children_count != 1 || !node->children[0])
        {
            fprintf(stderr, "[ERROR] Invalid negation node.\n");
            last_exit_status = 1;
            return;
        }

        // Évalue l'enfant du nœud de négation
        ast_eval(node->children[0]);

        // Inverse le code de retour
        last_exit_status = (last_exit_status == 0) ? 1 : 0;
        break;
    }

    default:
        fprintf(stderr, "le type de noeud n'est pas correct %d\n", node->type);
        break;
    }
}

void ast_pretty_print(struct ast *node, int depth)
{
    if (!node)
        return;

    // Affiche l'indentation
    for (int i = 0; i < depth; i++)
        printf(i == depth - 1 ? "|---" : "|   ");

    // Gère les différents types de nœuds
    switch (node->type)
    {
    case AST_LIST:
        printf("LIST\n");
        break;

    case AST_SIMPLE_COMMAND: {
        struct ast_command_data *data = (struct ast_command_data *)node->data;
        if (!data || !data->args || !data->args[0])
        {
            last_exit_status = 1;
            return;
        }

        // Calcule le nombre d'arguments
        size_t argc = 0;
        while (data->args[argc] != NULL)
        {
            argc++;
        }

        // Exécute la commande
        last_exit_status = execute_command((int)argc, data->args);
        break;
    }

    case AST_NEGATION: {
        printf("NEGATION\n");
        if (node->children_count > 0)
        {
            ast_pretty_print(node->children[0], depth + 1);
        }
        break;
    }

    case AST_IF: {
        printf("IF\n");

        // Affiche la condition
        printf("|   Condition:\n");
        struct ast_if_data *if_data = (struct ast_if_data *)node->data;
        if (if_data && if_data->condition)
        {
            ast_pretty_print(if_data->condition, depth + 1);
        }
        else
        {
            printf("|   |---(NULL)\n");
        }

        // Affiche le bloc then
        printf("|   Then:\n");
        if (if_data && if_data->then_branch)
        {
            ast_pretty_print(if_data->then_branch, depth + 1);
        }
        else
        {
            printf("|   |---(NULL)\n");
        }

        // Affiche le bloc else ou elif
        if (if_data && if_data->else_branch)
        {
            struct ast *else_branch = if_data->else_branch;

            // Vérifie si c'est un bloc elif
            if (else_branch->type == AST_IF)
            {
                printf("|   Elif:\n");
            }
            else
            {
                printf("|   Else:\n");
            }

            ast_pretty_print(else_branch, depth + 1);
        }
        break;
    }

    default:
        printf("UNKNOWN\n");
        break;
    }

    // Parcours les enfants restants
    for (size_t i = 0; i < node->children_count; i++)
    {
        ast_pretty_print(node->children[i], depth + 1);
    }
}
