#include "parser.h"

#include <stdio.h>
#include <stdlib.h>

#include "../lexer/lexer.h"
#include "../lexer/token.h"
#include "ast.h"

static struct token *consume_token(struct lexer *lexer)
{
    struct token *tok = lexer_next_token(lexer);
    if (!tok)
    {
        fprintf(stderr, "Erreur: Impossible de consommer un token.\n");
    }
    return tok;
}

static struct ast *create_ast_node(struct token *tok, enum parser_status *status)
{
    struct ast *node = ast_new();
    if (!node)
    {
        *status = PARSER_ERROR;
        return NULL;
    }
    node->token = *tok;
    node->children = NULL;
    node->children_count = 0;
    return node;
}

static struct ast *parse_command(struct lexer *lexer, struct token *tok, struct ast *root, struct ast **current_command, enum parser_status *status)
{
    if (lexer == NULL)
    {
        *status = PARSER_ERROR;
        return NULL;
    }
    
    struct ast *child = create_ast_node(tok, status);
    if (!child)
    {
        token_free(tok);
        ast_free(root);
        return NULL;
    }

    if (!*current_command)
    {
        *current_command = child;
        root->children = realloc(root->children, sizeof(struct ast *) * (root->children_count + 1));
        root->children[root->children_count++] = *current_command;
    }
    else
    {
        (*current_command)->children = realloc((*current_command)->children, sizeof(struct ast *) * ((*current_command)->children_count + 1));
        (*current_command)->children[(*current_command)->children_count++] = child;
    }

    return root;
}

static struct ast *parse_command_list(struct lexer *lexer, enum parser_status *status)
{
    struct ast *root = ast_new();
    if (!root)
    {
        *status = PARSER_ERROR;
        return NULL;
    }

    root->children = NULL;
    root->children_count = 0;

    struct ast *current_command = NULL;

    while (1)
    {
        struct token *tok = lexer_next_token(lexer);
        if (!tok || tok->type == TOKEN_EOF)
        {
            token_free(tok);
            break;
        }

        if (tok->type == TOKEN_WORD)
        {
            root = parse_command(lexer, tok, root, &current_command, status);
            if (*status == PARSER_ERROR)
                return NULL;
            continue;
        }

        if (tok->type == TOKEN_SEMICOLON)
        {
            token_free(tok);
            current_command = NULL;
            continue;
        }

        token_free(tok);
        *status = PARSER_ERROR;
        ast_free(root);
        return NULL;
    }

    return root;
}

struct ast *parser_parse(struct lexer *lexer, enum parser_status *status)
{
    *status = PARSER_OK;
    struct ast *root = parse_command_list(lexer, status);
    if (*status != PARSER_OK)
    {
        fprintf(stderr, "Erreur: Parsing échoué.\n");
        return NULL;
    }

    struct token *tok = consume_token(lexer);
    if (tok && tok->type != TOKEN_EOF)
    {
        *status = PARSER_ERROR;
        token_free(tok);
        ast_free(root);
        return NULL;
    }
    token_free(tok);

    return root;
}
