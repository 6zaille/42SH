#define _POSIX_C_SOURCE 200809L

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "../lexer/lexer.h"
#include "../lexer/token.h"
#include "../utils/variables_count.h"
#include "ast.h"
#include "parser.h"

int last_exit_status = 0;

struct ast *parse_and_or(struct lexer *lexer)
{
    struct ast *left = parse_change(lexer);
    if (!left)
        return NULL;

    struct token tok = lexer_peek(lexer);
    while (tok.type == TOKEN_AND || tok.type == TOKEN_OR)
    {
        lexer_pop(lexer);
        struct ast *right = parse_change(lexer);
        if (!right)
        {
            ast_free(left);
            return NULL;
        }

        struct ast *and_or_node = ast_create(AST_AND_OR);
        /*if (!and_or_node)
        {
            ast_free(left);
            ast_free(right);
            return NULL;
        }*/

        and_or_node->data = strdup(tok.type == TOKEN_AND ? "&&" : "||");
        and_or_node->children = malloc(2 * sizeof(struct ast *));
        /*if (!and_or_node->children)
        {
            ast_free(and_or_node);
            ast_free(left);
            ast_free(right);
            return NULL;
        }*/

        and_or_node->children[0] = left;
        and_or_node->children[1] = right;
        and_or_node->children_count = 2;

        left = and_or_node;
        tok = lexer_peek(lexer);
    }

    return left;
}
