#define _POSIX_C_SOURCE 200809L

#include "parser.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "../lexer/lexer.h"
#include "../lexer/token.h"

static struct ast *ast_create(enum ast_type type)
{
    struct ast *node = malloc(sizeof(*node));
    if (!node)
        return NULL;
    node->type = type;
    node->children_count = 0;
    node->children = NULL;
    node->data = NULL;
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

    struct token *tok = lexer_next_token(lexer);
    while (tok && tok->type == TOKEN_WORD)
    {
        data->args = append_arg(data->args, tok->value);
        token_free(tok);
        tok = lexer_next_token(lexer);
    }
    lexer_push_back(lexer, tok);
    cmd_node->data = data;
    return cmd_node;
}

static struct ast *parse_command_list(struct lexer *lexer) {
    struct ast *list_node = ast_create(AST_LIST);
    if (!list_node)
        return NULL;

    struct ast **children = NULL;
    size_t count = 0;

    while (1) {
        struct token *tok = lexer_next_token(lexer);
        if (!tok || tok->type == TOKEN_EOF) {
            token_free(tok);
            break;
        }

        lexer_push_back(lexer, tok);
        struct ast *cmd = parse_simple_command(lexer);
        if (!cmd) {
            token_free(tok);
            break;
        }

        children = safe_realloc(children, sizeof(struct ast *) * (count + 1));
        if (!children) {
            ast_free(cmd);
            break;
        }
        children[count++] = cmd;

        tok = lexer_next_token(lexer);
        if (tok->type == TOKEN_SEMICOLON) {
            token_free(tok);
            continue;
        }

        lexer_push_back(lexer, tok);
        break;
    }

    list_node->children = children;
    list_node->children_count = count;
    return list_node;
}

struct ast *parser_parse(struct lexer *lexer) {
    struct ast *root = parse_command_list(lexer);
    return root;
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
    free(node);
}
