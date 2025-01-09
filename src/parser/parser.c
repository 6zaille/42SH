#include <stdlib.h>
#include <stdio.h>
#include "parser.h"
#include "../lexer/lexer.h"
#include "../lexer/token.h"
#include "ast.h"

// DM-CM-)claration d'un token courant statique
static struct token *current_token = NULL;

// Fonction pour consommer le token courant et passer au suivant
static void consume_token(struct lexer *lexer)
{
    token_free(current_token);
    current_token = lexer_next_token(lexer);
}

// Fonction pour vM-CM-)rifier si le token courant est du type attendu
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
// Fonction pour analyser une liste de commandes
static struct ast *parse_command_list(struct lexer *lexer, enum parser_status *status)
{
    // Analyse une commande simple
    struct ast *root = parse_simple_command(lexer, status);
    if (*status != PARSER_OK)
        return NULL;

    // Tant que le token courant est un point-virgule ou une nouvelle ligne
    while (current_token->type == TOKEN_SEMICOLON || current_token->type == TOKEN_NEWLINE)
    {
        // Consomme le token courant
        consume_token(lexer);
        // Analyse la commande suivante
        struct ast *next_command = parse_simple_command(lexer, status);
        if (*status != PARSER_OK)
        {
            ast_free(root); // LibM-CM-(re la mM-CM-)moire de la racine si une erreur se produit
            return NULL;
        }

        // CrM-CM-)e un nouveau nM-EM-^Sud AST pour reprM-CM-)senter la liste de commandes
        struct ast *new_root = ast_new();
        new_root->token.type = TOKEN_SEMICOLON; // DM-CM-)finit le type du token comme point-virgule
        new_root->children = malloc(2 * sizeof(struct ast *)); // Alloue la mM-CM-)moire pour deux enfants
        new_root->children[0] = root; // Le premier enfant est la commande prM-CM-)cM-CM-)dente
        new_root->children[1] = next_command; // Le deuxiM-CM-(me enfant est la commande suivante
        new_root->children_count = 2; // Met M-CM-  jour le nombre d'enfants

        // Met M-CM-  jour la racine
        root = new_root;
    }

    return root; // Retourne la racine de l'AST
}

// Fonction principale pour analyser le lexer et retourner l'AST
struct ast *parser_parse(struct lexer *lexer, enum parser_status *status)
{
    *status = PARSER_OK; // Initialise le statut M-CM-  PARSER_OK
    current_token = lexer_next_token(lexer); // RM-CM-)cupM-CM-(re le premier token
    
    if (!current_token)
    {
        *status = PARSER_ERROR; // Met M-CM-  jour le statut M-CM-  PARSER_ERROR si aucun token n'est trouvM-CM-)
        return NULL;
    }

    struct ast *root = parse_command_list(lexer, status); // Analyse la liste de commandes

    if (current_token->type != TOKEN_EOF) // VM-CM-)rifie si le token courant est la fin du stream
        *status = PARSER_ERROR; // Met M-CM-  jour le statut M-CM-  PARSER_ERROR si ce n'est pas le cas

    token_free(current_token); // LibM-CM-(re la mM-CM-)moire du token courant
    return root; // Retourne la racine de l'AST
}