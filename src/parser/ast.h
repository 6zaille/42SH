#ifndef AST_H
#define AST_H

#include <err.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

#include "../execution/builtins.h"
#include "../execution/exec.h"
#include "../lexer/lexer.h"
#include "../lexer/token.h"
#include "ast.h"
#include "parser.h"

/**
 * This very simple AST structure should be sufficient for a simple AST.
 * It is however, NOT GOOD ENOUGH for more complicated projects, such as a
 * shell. Please read the project guide for some insights about other kinds of
 * ASTs.
 */
struct ast
{
    struct token token; // The kind of node we're dealing with
    ssize_t value; // If the node is a number, it stores its value
    struct ast **children; // Array of pointers to child nodes
    size_t children_count; // Number of children
};

/**
 ** \brief Allocates a new ast with the given type.
 */
struct ast *ast_new(void);

/**
 ** \brief Recursively frees the given ast.
 */
void ast_free(struct ast *ast);

void print_arbre(struct ast *node, int depth);

void eval_ast(struct ast *root);

#endif /* !AST_H */