#include <stdlib.h>
#include "parser.h"

static struct token *current_token;

static void consume_token(struct lexer *lexer)
{
    token_free(current_token);
    current_token = lexer_next_token(lexer);
}

static void expect(enum token_type type, enum parser_status *status, struct lexer *lexer)
{
    if (current_token->type != type)
    {
        *status = PARSER_ERROR;
        return;
    }
    consume_token(lexer);
}

struct ast *parse_simple_command(struct lexer *lexer, enum parser_status *status)
{
    if (current_token->type == TOKEN_WORD)
    {
        struct ast *node = ast_new(AST_SIMPLE_COMMAND);
        consume_token(lexer);
        return node;
    }

    *status = PARSER_ERROR;
    return NULL;
}

struct ast *parse_if_command(struct lexer *lexer, enum parser_status *status)
{
    expect(TOKEN_IF, status, lexer);
    if (*status != PARSER_OK)
        return NULL;

    struct ast *condition = parse_simple_command(lexer, status);
    if (*status != PARSER_OK)
        return NULL;

    expect(TOKEN_THEN, status, lexer);
    if (*status != PARSER_OK)
    {
        ast_free(condition);
        return NULL;
    }

    struct ast *then_branch = parse_simple_command(lexer, status);
    if (*status != PARSER_OK)
    {
        ast_free(condition);
        return NULL;
    }

    struct ast *else_branch = NULL;
    if (current_token->type == TOKEN_ELSE)
    {
        consume_token(lexer);
        else_branch = parse_simple_command(lexer, status);
        if (*status != PARSER_OK)
        {
            ast_free(condition);
            ast_free(then_branch);
            return NULL;
        }
    }

    expect(TOKEN_FI, status, lexer);
    if (*status != PARSER_OK)
    {
        ast_free(condition);
        ast_free(then_branch);
        ast_free(else_branch);
        return NULL;
    }

    struct ast *if_node = ast_new(AST_IF_COMMAND);
    if_node->left = condition;
    if_node->right = then_branch;

    if (else_branch)
        if_node->right->right = else_branch;

    return if_node;
}

struct ast *parser_parse(struct lexer *lexer, enum parser_status *status)
{
    current_token = lexer_next_token(lexer);

    struct ast *root = NULL;
    if (current_token->type == TOKEN_IF)
        root = parse_if_command(lexer, status);
    else if (current_token->type == TOKEN_WORD)
        root = parse_simple_command(lexer, status);

    if (current_token->type != TOKEN_EOF)
        *status = PARSER_ERROR;

    token_free(current_token);
    return root;
}
