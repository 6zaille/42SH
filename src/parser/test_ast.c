#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include "../lexer/lexer.h"
#include "../lexer/token.h"
#include "ast.h"
#include "parser.h"

int main(void)
{
    struct ast *root = ast_new();
    root->children_count = 1;
    root->children = malloc(sizeof(struct ast *) * root->children_count);

    root->children[0] = ast_new();
    root->children[0]->token.type = TOKEN_WORD;
    root->children[0]->token.value = "echo";
    root->children[0]->value = 0;
    root->children[0]->children_count = 1;

    root->children[0]->children =
        malloc(sizeof(struct ast *) * root->children[0]->children_count);
    root->children[0]->children[0] = ast_new();
    root->children[0]->children[0]->token.type = TOKEN_WORD;
    root->children[0]->children[0]->token.value = "pipi";

    //print_arbre(root);
    eval_ast(root);

    return 0;
}
