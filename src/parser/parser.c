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
    new_args[count] = strdup(arg);
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

    struct token filename = lexer_peek(lexer);
    if (filename.type != TOKEN_WORD)
    {
        fprintf(stderr, "erreur filename redirection\n");
        return NULL;
    }
    lexer_pop(lexer);

    struct redirection *redir = malloc(sizeof(*redir));
    redir->type = (tok.type == TOKEN_REDIRECT_IN) ? REDIR_IN : REDIR_OUT;
    redir->filename = strdup(filename.value);
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
        tok = lexer_pop(lexer);
    }

    cmd_node->data = data;
    return cmd_node;
}

static const char *token_type_to_string(enum token_type type)
{
    switch (type)
    {
    case TOKEN_IF:
        return "if";
    case TOKEN_THEN:
        return "then";
    case TOKEN_ELSE:
        return "else";
    case TOKEN_FI:
        return "fi";
    default:
        return "UNKNOWN";
    }
}

static void print_token_names(struct lexer *lexer)
{
    struct token tok;
    tok = lexer_peek(lexer);
    while (tok.type != TOKEN_EOF && tok.type != TOKEN_NEWLINE)
    {
        printf("%s\n", token_type_to_string(tok.type));
        free(tok.value);
        tok = lexer_pop(lexer);
    }
}

static struct ast *parse_change(struct lexer *lexer)
{
    struct token tok = lexer_peek(lexer);

    // Cas de négation
    if (tok.type == TOKEN_NEGATION)
    {
        lexer_pop(lexer);
        struct ast *child = parse_change(lexer);
        if (!child)
            return NULL;

        struct ast *negation_node = ast_create(AST_NEGATION);
        if (!negation_node)
        {
            ast_free(child);
            return NULL;
        }

        negation_node->children = malloc(sizeof(struct ast *));
        if (!negation_node->children)
        {
            ast_free(negation_node);
            ast_free(child);
            return NULL;
        }

        negation_node->children[0] = child;
        negation_node->children_count = 1;
        return negation_node;
    }

    // Analyse d'un pipeline
    struct ast *pipeline_node = parse_pipeline(lexer);
    if (pipeline_node)
        return pipeline_node;

    // Cas par défaut : commande simple
    return parse_simple_command(lexer);
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
        struct ast *command_node = parse_change(lexer);
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
            print_token_names(lexer);
            fflush(stdout);
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
        {
            lexer_pop(lexer);
            break;
        }

        if (tok.type == TOKEN_EOF || tok.type == TOKEN_FI)
        {
            fprintf(stderr, "mauvais token fin dans if condition\n");
            ast_free(condition);
            return NULL;
        }

        struct ast *cmd = parse_command_list(lexer);
        if (!cmd)
        {
            fprintf(stderr, "mauvaise commande dans if condition\n");
            ast_free(condition);
            return NULL;
        }

        condition->children =
            realloc(condition->children,
                    sizeof(struct ast *) * (condition->children_count + 1));
        if (!condition->children)
        {
            perror("realloc");
            ast_free(condition);
            return NULL;
        }
        condition->children[condition->children_count++] = cmd;

        tok = lexer_peek(lexer);
        if (tok.type == TOKEN_SEMICOLON || tok.type == TOKEN_NEWLINE)
        {
            lexer_pop(lexer);
        }
        else if (tok.type != TOKEN_THEN)
        {
            fprintf(stderr, "token anormal dans if'%s'\n",
                    tok.value ? tok.value : "NULL");
            ast_free(condition);
            return NULL;
        }
    }

    return condition;
}

static struct ast *parse_then(struct lexer *lexer)
{
    struct ast *condition = ast_create(AST_LIST);
    if (!condition)
        return NULL;

    condition->children = NULL;
    condition->children_count = 0;

    while (1)
    {
        struct token tok = lexer_peek(lexer);
        if (tok.type == TOKEN_ELSE)
        {
            lexer_pop(lexer);
            break;
        }

        if (tok.type == TOKEN_EOF || tok.type == TOKEN_FI)
        {
            fprintf(stderr, "probleme de fin dans parse_then\n");
            ast_free(condition);
            return NULL;
        }

        struct ast *cmd = parse_command_list(lexer);
        if (!cmd)
        {
            fprintf(stderr, "commande incorrect\n");
            ast_free(condition);
            return NULL;
        }

        condition->children =
            realloc(condition->children,
                    sizeof(struct ast *) * (condition->children_count + 1));
        if (!condition->children)
        {
            perror("realloc");
            ast_free(condition);
            return NULL;
        }
        condition->children[condition->children_count++] = cmd;

        tok = lexer_peek(lexer);
        if (tok.type == TOKEN_SEMICOLON || tok.type == TOKEN_NEWLINE)
        {
            lexer_pop(lexer);
        }
        else if (tok.type != TOKEN_THEN)
        {
            fprintf(stderr, "token anormal dans then'%s'\n",
                    tok.value ? tok.value : "NULL");
            ast_free(condition);
            return NULL;
        }
    }

    return condition;
}

static struct ast *parse_else(struct lexer *lexer)
{
    struct ast *condition = ast_create(AST_LIST);
    if (!condition)
        return NULL;

    condition->children = NULL;
    condition->children_count = 0;

    while (1)
    {
        struct token tok = lexer_peek(lexer);
        if (tok.type == TOKEN_FI)
        {
            lexer_pop(lexer);
            break;
        }

        if (tok.type == TOKEN_EOF)
        {
            fprintf(stderr, "probleme de fin dans else\n");
            ast_free(condition);
            return NULL;
        }

        struct ast *cmd = parse_command_list(lexer);
        if (!cmd)
        {
            fprintf(stderr, "commande invalide dans parse_else\n");
            ast_free(condition);
            return NULL;
        }

        condition->children =
            realloc(condition->children,
                    sizeof(struct ast *) * (condition->children_count + 1));
        if (!condition->children)
        {
            perror("realloc");
            ast_free(condition);
            return NULL;
        }
        condition->children[condition->children_count++] = cmd;

        tok = lexer_peek(lexer);
        if (tok.type == TOKEN_SEMICOLON || tok.type == TOKEN_NEWLINE)
        {
            lexer_pop(lexer);
        }
        else if (tok.type != TOKEN_THEN)
        {
            fprintf(stderr, "token anormal dans else'%s'\n",
                    tok.value ? tok.value : "NULL");
            ast_free(condition);
            return NULL;
        }
    }

    return condition;
}

static struct ast *parse_if_statement(struct lexer *lexer)
{
    struct token tok = lexer_peek(lexer);
    if (tok.type != TOKEN_IF)
    {
        fprintf(stderr, "erreur dans if_statement '%s'\n",
                tok.value ? tok.value : "NULL");
        return NULL;
    }
    lexer_pop(lexer);

    struct ast *condition = parse_if_condition(lexer);
    if (!condition)
        return NULL;
    struct ast *then_branch = parse_then(lexer);
    if (!then_branch)
    {
        ast_free(condition);
        return NULL;
    }
    struct ast *else_branch = parse_else(lexer);
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
    struct ast *first_command = parse_simple_command(lexer);
    if (!first_command)
        return NULL;

    struct token tok = lexer_peek(lexer);
    if (tok.type != TOKEN_PIPE)
        return first_command;

    struct ast *pipeline_node = ast_create(AST_PIPELINE);
    if (!pipeline_node)
    {
        ast_free(first_command);
        return NULL;
    }

    pipeline_node->children = malloc(sizeof(struct ast *));
    if (!pipeline_node->children)
    {
        ast_free(pipeline_node);
        ast_free(first_command);
        return NULL;
    }

    pipeline_node->children[0] = first_command;
    pipeline_node->children_count = 1;

    while (tok.type == TOKEN_PIPE)
    {
        lexer_pop(lexer);
        struct ast *next_command = parse_simple_command(lexer);
        if (!next_command)
        {
            ast_free(pipeline_node);
            return NULL;
        }

        struct ast **new_children = realloc(
            pipeline_node->children,
            sizeof(struct ast *) * (pipeline_node->children_count + 1));
        if (!new_children)
        {
            ast_free(next_command);
            ast_free(pipeline_node);
            return NULL;
        }

        pipeline_node->children = new_children;
        pipeline_node->children[pipeline_node->children_count++] = next_command;

        tok = lexer_peek(lexer);
    }

    return pipeline_node;
}