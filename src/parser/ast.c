#include "ast.h"

#include "parser.h"

int last_exit_status = 0;
int loop_running = 1;
/*
void ast_eval_for(struct ast *node)
{
    if (!node || node->type != AST_FOR || node->children_count < 2)
    {
        fprintf(stderr, "AST_FOR: Invalid node structure\n");
        last_exit_status = 1;
        return;
    }

    // Récupération des parties du nœud AST_FOR
    char *variable_name = (char *)node->data;
    struct ast *values = node->children[0]; // Les valeurs de la boucle
    struct ast *body = node->children[1]; // Le corps de la boucle

    // Parcourir les valeurs
    for (size_t i = 0; i < values->children_count; i++)
    {
        struct ast *value_node = values->children[i];
        if (!value_node || !value_node->data)
        {
            fprintf(stderr, "AST_FOR: Invalid value node\n");
            continue;
        }

        // Affecter la valeur actuelle à la variable
        set_variable(variable_name, (char *)value_node->data);

        // Évaluer le corps de la boucle pour chaque itération
        for (size_t j = 0; j < body->children_count; j++)
        {
            ast_eval(body->children[j]);
        }

        // Interruption de la boucle si nécessaire
        if (loop_running == 0)
        {
            break;
        }
    }
}
*/
void ast_eval_simple_command(struct ast *node)
{
    struct ast_command_data *data = (struct ast_command_data *)node->data;
    if (!data || !data->args || !data->args[0])
    {
        last_exit_status = 1;
        return;
    }
    const char *cmd = data->args[0];

    if (strcmp(cmd, "break") == 0)
    {
        loop_running = 0;
        last_exit_status = 0;
        return;
    }
    int argc = 0;
    while (data->args[argc] != NULL)
    {
        argc++;
    }

    last_exit_status = execute_command(argc, data->args);
}

void ast_eval_pipeline(struct ast *node)
{
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
}

void ast_eval_while(struct ast *node)
{
    struct ast *condition = node->children[0];
    struct ast *body = node->children[1];

    while (loop_running)
    {
        ast_eval(condition);

        if (last_exit_status != 0)
        {
            break;
        }

        ast_eval(body);
    }
    loop_running = 1;
    last_exit_status = 0;
}

void ast_eval_until(struct ast *node)
{
    struct ast *condition = node->children[0];
    struct ast *body = node->children[1];

    while (1)
    {
        ast_eval(condition);

        if (last_exit_status == 0)
        {
            break;
        }

        ast_eval(body);

        if (loop_running == 0)
        {
            loop_running = 1;
            break;
        }
    }
}

void ast_eval_if(struct ast *node)
{
    struct ast_if_data *data = (struct ast_if_data *)node->data;
    /*if (!data)
    {
        return;
    }*/
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
    /*else if (!data->then_branch && !data->else_branch)
    {
        fprintf(stderr, "else or then missing\n");
    }*/
}

void ast_eval_negation(struct ast *node)
{
    if (node->children_count != 1 || !node->children[0])
    {
        fprintf(stderr, "pipeline incorrect\n");
        last_exit_status = 1;
        return;
    }
    ast_eval(node->children[0]);
    last_exit_status = (last_exit_status == 0) ? 1 : 0;
}

void ast_eval_and_or(struct ast *node)
{
    if (strcmp(node->data, "&&") == 0)
    {
        ast_eval(node->children[0]);
        if (last_exit_status == 0)
            ast_eval(node->children[1]);
    }
    else if (strcmp(node->data, "||") == 0)
    {
        ast_eval(node->children[0]);
        if (last_exit_status != 0)
            ast_eval(node->children[1]);
    }
}

void ast_eval(struct ast *node)
{
    /*if (!node)
    {
        return;
    }*/
    switch (node->type)
    {
    case AST_SIMPLE_COMMAND:
        ast_eval_simple_command(node);
        break;
    case AST_LIST:
        for (size_t i = 0; i < node->children_count; i++)
        {
            ast_eval(node->children[i]);
        }
        break;
    case AST_PIPELINE:
        ast_eval_pipeline(node);
        break;
    case AST_WHILE:
        ast_eval_while(node);
        break;
    case AST_UNTIL:
        ast_eval_until(node);
        break;
    case AST_IF:
        ast_eval_if(node);
        break;
    case AST_NEGATION:
        ast_eval_negation(node);
        break;
    case AST_AND_OR:
        ast_eval_and_or(node);
        break;
    /*case AST_FOR:
        ast_eval_for(node);
        break;*/
    default:
        fprintf(stderr, "le type de noeud n'est pas correct %d\n", node->type);
        break;
    }
}

/*

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

    case AST_SIMPLE_COMMAND: {
        struct ast_command_data *data = (struct ast_command_data *)node->data;
        if (!data || !data->args || !data->args[0])
        {
            last_exit_status = 1;
            return;
        }

        size_t argc = 0;
        while (data->args[argc] != NULL)
        {
            argc++;
        }

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

        printf("|   Then:\n");
        if (if_data && if_data->then_branch)
        {
            ast_pretty_print(if_data->then_branch, depth + 1);
        }
        else
        {
            printf("|   |---(NULL)\n");
        }

        if (if_data && if_data->else_branch)
        {
            struct ast *else_branch = if_data->else_branch;

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

    for (size_t i = 0; i < node->children_count; i++)
    {
        ast_pretty_print(node->children[i], depth + 1);
    }
}
*/
