#include <stdlib.h>
#include <stdio.h>
#include "parser.h"
#include "../lexer/lexer.h"
#include "../lexer/token.h"
#include "ast.h"

static struct token *consume_token(struct lexer *lexer)
{
    struct token *tok = lexer_next_token(lexer);
    if (!tok)
    {
        fprintf(stderr, "Erreur: Impossible de consommer un token.\n");
        return NULL;
    }
    return tok;
}

/*static void expect_token(struct token *tok, enum token_type type, enum parser_status *status)
{
    if (!tok || tok->type != type)
    {
        *status = PARSER_ERROR;
    }
}
*/
static struct ast *parse_simple_command(struct lexer *lexer, enum parser_status *status)
{
    struct ast *command_node = ast_new();
    if (!command_node)
    {
        *status = PARSER_ERROR;
        return NULL;
    }

    struct token *tok = consume_token(lexer);
    if (!tok || tok->type != TOKEN_WORD)
    {
        *status = PARSER_ERROR;
        ast_free(command_node);
        return NULL;
    }

    command_node->token = *tok;
    command_node->children = NULL;
    command_node->children_count = 0;

    struct ast *current_parent = command_node;

    while ((tok = consume_token(lexer)) && tok->type == TOKEN_WORD)
    {
        struct ast *arg_node = ast_new();
        if (!arg_node)
        {
            *status = PARSER_ERROR;
            ast_free(command_node);
            return NULL;
        }

        arg_node->token = *tok;

        current_parent->children = realloc(current_parent->children, sizeof(struct ast *) * (current_parent->children_count + 1));
        current_parent->children[current_parent->children_count++] = arg_node;
        current_parent = arg_node;
    }

    return command_node;
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

    struct ast *first_command = parse_simple_command(lexer, status);
    if (*status != PARSER_OK)
    {
        ast_free(root);
        return NULL;
    }

    root->children = malloc(sizeof(struct ast *));
    root->children[0] = first_command;
    root->children_count = 1;

    struct token *tok;
    while ((tok = consume_token(lexer)) && (tok->type == TOKEN_SEMICOLON || tok->type == TOKEN_NEWLINE))
    {
        if ((tok = consume_token(lexer)) && tok->type == TOKEN_EOF)
        {
            break;
        }

        struct ast *next_command = parse_simple_command(lexer, status);
        if (*status != PARSER_OK)
        {
            ast_free(root);
            return NULL;
        }

        root->children = realloc(root->children, sizeof(struct ast *) * (root->children_count + 1));
        root->children[root->children_count++] = next_command;
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
        fprintf(stderr, "Erreur: Flux non terminé correctement.\n");
    }

    return root;
}
