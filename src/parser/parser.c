#define _POSIX_C_SOURCE 200809L

#include "parser.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "../lexer/lexer.h"
#include "../lexer/token.h"
#include "ast.h"

static struct ast *ast_create(enum ast_type type)
{
    struct ast *node = malloc(sizeof(*node));
    if (!node)
    {
        return NULL;
    }
    node->type = type;
    node->children_count = 0;
    node->children = NULL;
    node->data = NULL;
    return node;
}

static struct ast *ast_create_if(struct ast *condition, struct ast *then_branch,
                                 struct ast *else_branch)
{
    struct ast *node = ast_create(AST_IF);
    if (!node)
    {
        return NULL;
    }
    struct ast_if_data *data = malloc(sizeof(*data));
    if (!data)
    {
        free(node);
        return NULL;
    }
    data->condition = condition;
    data->then_branch = then_branch;
    data->else_branch = else_branch;
    node->data = data;

    return node;
}

static void *safe_realloc(void *ptr, size_t new_size)
{
    void *new_ptr = realloc(ptr, new_size);
    if (!new_ptr)
        free(ptr);
    return new_ptr;
}

static char **append_arg(char **args, const char *arg)
{
    size_t count = 0;
    while (args && args[count])
        count++;
    char **new_args = safe_realloc(args, (count + 2) * sizeof(char *));
    if (!new_args)
        return NULL;

    new_args[count] = malloc(strlen(arg) + 1);
    if (!new_args[count])
        return NULL;

    strcpy(new_args[count], arg);
    new_args[count + 1] = NULL;

    return new_args;
}

static struct redirection *parse_redirection(struct lexer *lexer)
{
    struct token tok = lexer_peek(lexer);
    if (tok.type != TOKEN_REDIRECT_IN && tok.type != TOKEN_REDIRECT_OUT)
    {
        return NULL;
    }
    lexer_pop(lexer);
    free(tok.value);

    struct token filename = lexer_peek(lexer);
    if (filename.type != TOKEN_WORD)
    {
        fprintf(stderr, "error\n");
        return NULL;
    }
    lexer_pop(lexer);
    struct redirection *redir = malloc(sizeof(*redir));
    if (!redir)
        return NULL;

    redir->type = (tok.type == TOKEN_REDIRECT_IN) ? REDIR_IN : REDIR_OUT;

    redir->filename = malloc(strlen(filename.value) + 1);
    if (!redir->filename)
    {
        free(redir);
        return NULL;
    }
    strcpy(redir->filename, filename.value);

    free(filename.value);
    return redir;
}

static struct ast *parse_simple_command(struct lexer *lexer)
{
    struct ast *cmd_node = ast_create(AST_SIMPLE_COMMAND);
    if (!cmd_node)
        return NULL;

    struct ast_command_data *data = malloc(sizeof(*data));
    if (!data)
    {
        free(cmd_node);
        return NULL;
    }
    data->args = NULL;
    data->redirections = NULL;
    data->redirection_count = 0;

    struct token tok = lexer_peek(lexer);
    while (tok.type == TOKEN_WORD
           || (tok.type >= TOKEN_REDIRECT_IN && tok.type <= TOKEN_REDIRECT_RW))
    {
        lexer_pop(lexer);
        
        if (tok.type == TOKEN_WORD)
        {
            data->args = append_arg(data->args, tok.value);
        }
        else
        {
            struct redirection *redir = parse_redirection(lexer);
            if (redir)
            {
                data->redirections =
                    realloc(data->redirections,
                            sizeof(*data->redirections)
                                * (data->redirection_count + 1));
                if (!data->redirections)
                {
                    perror("realloc");
                    exit(EXIT_FAILURE);
                }
                data->redirections[data->redirection_count++] = *redir;
                free(redir);
            }
        }
        free(tok.value);
        tok = lexer_peek(lexer);
    }

    cmd_node->data = data;
    return cmd_node;
}

static struct ast *parse_command_or_pipeline(struct lexer *lexer)
{
    struct ast *pipeline_node = ast_create(AST_PIPELINE);
    if (!pipeline_node)
        return NULL;

    struct ast **commands = NULL;
    size_t count = 0;

    while (1)
    {
        struct ast *command_node = parse_simple_command(lexer);
        if (!command_node)
        {
            ast_free(pipeline_node);
            return NULL;
        }

        commands = realloc(commands, sizeof(struct ast *) * (count + 1));
        if (!commands)
        {
            ast_free(command_node);
            ast_free(pipeline_node);
            return NULL;
        }

        commands[count++] = command_node;

        struct token tok = lexer_peek(lexer);
        if (tok.type == TOKEN_PIPE)
        {
            lexer_pop(lexer);
        }
        else
        {
            break;
        }
    }

    pipeline_node->children = commands;
    pipeline_node->children_count = count;

    if (count == 1)
    {
        struct ast *single_command = commands[0];
        free(pipeline_node->children);
        free(pipeline_node);
        return single_command;
    }

    return pipeline_node;
}

static struct ast *parse_command_list(struct lexer *lexer)
{
    struct ast *list_node = ast_create(AST_LIST);
    if (!list_node)
        return NULL;

    struct ast **commands = NULL;
    size_t count = 0;

    while (1)
    {
        struct ast *command_node = parse_command_or_pipeline(lexer);
        if (!command_node)
        {
            ast_free(list_node);
            return NULL;
        }

        commands = realloc(commands, sizeof(struct ast *) * (count + 1));
        if (!commands)
        {
            ast_free(command_node);
            ast_free(list_node);
            return NULL;
        }

        commands[count++] = command_node;

        struct token tok = lexer_peek(lexer);
        if (tok.type == TOKEN_SEMICOLON)
        {
            lexer_pop(lexer);
        }
        else if (tok.type == TOKEN_EOF || tok.type == TOKEN_NEWLINE)
        {
            break;
        }
        else
        {
            fprintf(stderr, "token faux '%s'\n", tok.value);
            ast_free(list_node);
            return NULL;
        }
    }
    
    list_node->children = commands;
    list_node->children_count = count;
    return list_node;
}

static struct ast *parse_if_condition(struct lexer *lexer)
{
    struct ast *condition = ast_create(AST_LIST);
    if (!condition)
        return NULL;

    condition->children = NULL;
    condition->children_count = 0;

    while (1)
    {
        struct token tok = lexer_peek(lexer);
        if (tok.type == TOKEN_THEN)
            break;

        struct ast *cmd = parse_command_or_pipeline(lexer);
        if (!cmd)
        {
            ast_free(condition);
            return NULL;
        }

        condition->children = realloc(condition->children, sizeof(struct ast *) * (condition->children_count + 1));
        if (!condition->children)
        {
            ast_free(cmd);
            ast_free(condition);
            return NULL;
        }
        condition->children[condition->children_count++] = cmd;

        tok = lexer_peek(lexer);
        if (tok.type != TOKEN_SEMICOLON && tok.type != TOKEN_NEWLINE && tok.type != TOKEN_THEN)
        {
            fprintf(stderr, "token faux '%s'\n", tok.value);
            ast_free(condition);
            return NULL;
        }

        if (tok.type != TOKEN_THEN)
            lexer_pop(lexer);
    }

    return condition;
}




static struct ast *parse_if_statement(struct lexer *lexer)
{
    struct token tok = lexer_peek(lexer);
    if (tok.type != TOKEN_IF)
    {
        fprintf(stderr, "'if' est attendu\n");
        return NULL;
    }
    lexer_pop(lexer);
    free(tok.value);

    struct ast *condition = parse_if_condition(lexer);
    if (!condition)
    {
        return NULL;
    }

    tok = lexer_peek(lexer);
    if (tok.type != TOKEN_THEN)
    {
        fprintf(stderr, "'then' est attendu\n");
        ast_free(condition);
        return NULL;
    }
    lexer_pop(lexer);
    free(tok.value);

    struct ast *then_branch = parse_command_list(lexer);
    if (!then_branch)
    {
        ast_free(condition);
        return NULL;
    }

    while (1)
    {
        tok = lexer_peek(lexer);
        if (tok.type == TOKEN_ELSE || tok.type == TOKEN_FI)
            break;
        if (tok.type != TOKEN_NEWLINE)
        {
            fprintf(stderr, "token faux '%s'\n", tok.value);
            ast_free(condition);
            ast_free(then_branch);
            return NULL;
        }
        lexer_pop(lexer);
        free(tok.value);
    }

    struct ast *else_branch = NULL;
    if (tok.type == TOKEN_ELSE)
    {
        lexer_pop(lexer);
        free(tok.value);

        else_branch = parse_command_list(lexer);
        if (!else_branch)
        {
            ast_free(condition);
            ast_free(then_branch);
            return NULL;
        }
    }

    tok = lexer_peek(lexer);
    if (tok.type != TOKEN_FI)
    {
        fprintf(stderr, "'fi' est attendu\n");
        ast_free(condition);
        ast_free(then_branch);
        ast_free(else_branch);
        return NULL;
    }
    lexer_pop(lexer);
    free(tok.value);

    return ast_create_if(condition, then_branch, else_branch);
}





struct ast *parser_parse(struct lexer *lexer)
{
    struct token tok = lexer_peek(lexer);

    if (tok.type == TOKEN_IF)
    {
        return parse_if_statement(lexer);
    }

    return parse_command_list(lexer);
}

void ast_free(struct ast *node)
{
    if (!node)
        return;
    for (size_t i = 0; i < node->children_count; i++)
    {
        ast_free(node->children[i]);
    }
    free(node->children);

    if (node->type == AST_SIMPLE_COMMAND)
    {
        struct ast_command_data *data = node->data;
        if (data)
        {
            for (size_t i = 0; data->args && data->args[i]; i++)
            {
                free(data->args[i]);
            }
            free(data->args);
            free(data);
        }
    }
    else if (node->type == AST_IF)
    {
        struct ast_if_data *data = node->data;
        if (data)
        {
            ast_free(data->condition);
            ast_free(data->then_branch);
            ast_free(data->else_branch);
            free(data);
        }
    }
    free(node);
}

struct ast *parse_pipeline(struct lexer *lexer)
{
    struct ast *pipeline_node = ast_create(AST_PIPELINE);
    struct ast **commands = NULL;
    size_t count = 0;

    while (1)
    {
        struct ast *command = parse_simple_command(lexer);
        if (!command)
            break;

        commands = realloc(commands, (count + 1) * sizeof(struct ast *));
        commands[count++] = command;

        struct token tok = lexer_peek(lexer);
        if (tok.type != TOKEN_PIPE)
        {
            break;
        }
        lexer_pop(lexer);
        free(tok.value);
        tok = lexer_peek(lexer);
        if (tok.type == TOKEN_NEGATION)
        {
            lexer_pop(lexer);
            free(tok.value);

            struct ast *negation_node = ast_create(AST_NEGATION);
            negation_node->children = malloc(sizeof(struct ast *));
            if (!negation_node->children)
            {
                perror("malloc");
                free(negation_node);
                ast_free(pipeline_node);
                return NULL;
            }
            negation_node->children[0] = pipeline_node;
            negation_node->children_count = 1;
            return negation_node;
        }
    }

    pipeline_node->children = commands;
    pipeline_node->children_count = count;
    return pipeline_node;
}
