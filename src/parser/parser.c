#include <stdlib.h>
#include <stdio.h>
#include "parser.h"
#include "../lexer/lexer.h"
#include "../lexer/token.h"
#include "ast.h"

// Déclaration d'un token courant statique
static struct token *current_token = NULL;

// Fonction pour consommer le token courant et passer au suivant
static void consume_token(struct lexer *lexer)
{
    token_free(current_token);
    current_token = lexer_next_token(lexer);
}

// Fonction pour vérifier si le token courant est du type attendu
static void expect(enum token_type type, enum parser_status *status, struct lexer *lexer)
{
    if (current_token->type != type)
    {
        *status = PARSER_ERROR;
        return;
    }
    consume_token(lexer);
}

static struct ast *parse_simple_command(struct lexer *lexer, enum parser_status *status)
{
    struct ast *node = ast_new();
    if (!node)
    {
        *status = PARSER_ERROR;
        return NULL;
    }

    if (current_token->type != TOKEN_WORD)
    {
        *status = PARSER_ERROR;
        ast_free(node);
        return NULL;
    }

    node->token = *current_token;
    consume_token(lexer);

    // Collect additional WORDs as children (arguments)
    while (current_token->type == TOKEN_WORD)
    {
        struct ast *child = ast_new();
        if (!child)
        {
            *status = PARSER_ERROR;
            ast_free(node);
            return NULL;
        }
        child->token = *current_token;
        node->children = realloc(node->children, sizeof(struct ast *) * (node->children_count + 1));
        node->children[node->children_count++] = child;
        consume_token(lexer);
    }

    return node;
}

static struct ast *parse_command_list(struct lexer *lexer, enum parser_status *status)
{
    struct ast *root = parse_simple_command(lexer, status);
    if (*status != PARSER_OK)
        return NULL;

    while (current_token->type == TOKEN_SEMICOLON || current_token->type == TOKEN_NEWLINE)
    {
        consume_token(lexer);
        struct ast *next_command = parse_simple_command(lexer, status);
        if (*status != PARSER_OK)
        {
            ast_free(root);
            return NULL;
        }

        struct ast *new_root = ast_new();
        new_root->token.type = TOKEN_SEMICOLON;
        new_root->children = malloc(2 * sizeof(struct ast *));
        new_root->children[0] = root;
        new_root->children[1] = next_command;
        new_root->children_count = 2;

        root = new_root;
    }

    return root;
}

struct ast *parser_parse(struct lexer *lexer, enum parser_status *status)
{
    *status = PARSER_OK;
    current_token = lexer_next_token(lexer);
    
    if (!current_token)
    {
        *status = PARSER_ERROR;
        return NULL;
    }

    struct ast *root = parse_command_list(lexer, status);

    if (current_token->type != TOKEN_EOF)
        *status = PARSER_ERROR;

    token_free(current_token);
    return root;
}
