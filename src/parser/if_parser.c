#define _POSIX_C_SOURCE 200809L

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "../lexer/lexer.h"
#include "../lexer/token.h"
#include "ast.h"
#include "parser.h"

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
        if (tok.type == TOKEN_IF)
        {
            struct ast *nested_if = parse_if_statement(lexer);
            if (!nested_if)
            {
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

            condition->children[condition->children_count++] = nested_if;
        }
        if (tok.type == TOKEN_EOF || tok.type == TOKEN_FI)
        {
            ast_free(condition);
            return NULL;
        }

        struct ast *cmd = parse_command_list(lexer);
        if (!cmd)
        {
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
        else if (tok.type != TOKEN_THEN && tok.type != TOKEN_IF)
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
        if (tok.type == TOKEN_ELSE || tok.type == TOKEN_FI
            || tok.type == TOKEN_ELIF)
        {
            break;
        }

        if (tok.type == TOKEN_EOF)
        {
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
        else if (tok.type == TOKEN_ELSE)
        {
            break;
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
            break;
        }

        if (tok.type == TOKEN_EOF)
        {
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
        else if (tok.type == TOKEN_FI)
        {
            break;
        }
    }

    return condition;
}

struct ast *parse_if_statement(struct lexer *lexer)
{
    struct token tok = lexer_peek(lexer);
    if (tok.type != TOKEN_IF)
    {
        fprintf(stderr, "Syntax error: expected 'if', got '%s'\n",
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

    struct ast *else_branch = NULL;

    while (1)
    {
        tok = lexer_peek(lexer);

        if (tok.type == TOKEN_ELIF)
        {
            lexer_pop(lexer);

            struct ast *elif_condition = parse_if_condition(lexer);
            if (!elif_condition)
            {
                ast_free(condition);
                ast_free(then_branch);
                return NULL;
            }

            struct ast *elif_then = parse_then(lexer);
            if (!elif_then)
            {
                ast_free(condition);
                ast_free(then_branch);
                ast_free(elif_condition);
                return NULL;
            }

            struct ast *elif_node =
                ast_create_if(elif_condition, elif_then, NULL);
            if (!elif_node)
            {
                ast_free(condition);
                ast_free(then_branch);
                ast_free(elif_condition);
                ast_free(elif_then);
                return NULL;
            }

            if (!else_branch)
            {
                else_branch = elif_node;
            }
            else
            {
                struct ast_if_data *current_else_data =
                    (struct ast_if_data *)else_branch->data;
                while (current_else_data->else_branch
                       && current_else_data->else_branch->type == AST_IF)
                {
                    current_else_data =
                        (struct ast_if_data *)
                            current_else_data->else_branch->data;
                }
                current_else_data->else_branch = elif_node;
            }
        }
        else
            break;
    }

    tok = lexer_peek(lexer);
    if (tok.type == TOKEN_ELSE)
    {
        lexer_pop(lexer);

        struct ast *else_body = parse_else(lexer);
        if (!else_body)
        {
            ast_free(condition);
            ast_free(then_branch);
            ast_free(else_branch);
            return NULL;
        }

        if (!else_branch)
        {
            else_branch = else_body;
        }
        else
        {
            struct ast_if_data *current_else_data =
                (struct ast_if_data *)else_branch->data;
            while (current_else_data->else_branch
                   && current_else_data->else_branch->type == AST_IF)
            {
                current_else_data =
                    (struct ast_if_data *)current_else_data->else_branch->data;
            }
            current_else_data->else_branch = else_body;
        }
    }

    tok = lexer_peek(lexer);
    if (tok.type != TOKEN_FI)
    {
        fprintf(stderr, "Syntax error: expected 'fi', got '%s'\n",
                tok.value ? tok.value : "NULL");
        ast_free(condition);
        ast_free(then_branch);
        ast_free(else_branch);
        return NULL;
    }
    lexer_pop(lexer);

    return ast_create_if(condition, then_branch, else_branch);
}
