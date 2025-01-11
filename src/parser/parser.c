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
            // Si c'est un mot, l'ajouter comme un nouvel argument ou commande
            struct ast *child = ast_new();
            if (!child)
            {
                *status = PARSER_ERROR;
                token_free(tok);
                ast_free(root);
                return NULL;
            }

            child->token = *tok;
            tok->value = NULL;
            child->children = NULL;
            child->children_count = 0;

            if (!current_command)
            {
                // Si aucune commande actuelle, c'est une nouvelle commande
                current_command = child;
                root->children = realloc(root->children, sizeof(struct ast *) * (root->children_count + 1));
                root->children[root->children_count++] = current_command;
            }
            else
            {
                // Sinon, c'est un argument de la commande actuelle
                current_command->children = realloc(current_command->children, sizeof(struct ast *) * (current_command->children_count + 1));
                current_command->children[current_command->children_count++] = child;
            }
            token_free(tok);
            continue;
        }

        if (tok->type == TOKEN_SEMICOLON)
        {
            // Si un point-virgule est rencontré, réinitialiser la commande courante
            token_free(tok);
            current_command = NULL;
            continue;
        }

        // Gestion des erreurs
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
