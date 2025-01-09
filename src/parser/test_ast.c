#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include "../lexer/lexer.h"
#include "../lexer/token.h"
#include "ast.h"
#include "parser.h"

int main(void)
{
    printf("Test AST:\n");
    printf("---------------------------------------------------------------------\n");
    printf("simple test avec echo pipi\n");

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
    root->children[0]->children[0]->token.value = "toto";
    print_arbre(root, 0);
    eval_ast(root);

    printf("---------------------------------------------------------------------\n");
    printf("simple test avec echo pipi miaou\n");

    // Add another node under the node with token "pipi"
    root->children[0]->children[0]->children_count = 1;
    root->children[0]->children[0]->children =
        malloc(sizeof(struct ast *) * root->children[0]->children[0]->children_count);
    root->children[0]->children[0]->children[0] = ast_new();
    root->children[0]->children[0]->children[0]->token.type = TOKEN_WORD;
    root->children[0]->children[0]->children[0]->token.value = "chat";
    root->children[0]->children[0]->children[0]->children_count = 0;
    root->children[0]->children[0]->children[0]->value = 0;

    //print_arbre(root);
    eval_ast(root);

    return 0;
}