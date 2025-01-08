#ifndef AST_H
#define AST_H

#include <stdlib.h>

enum ast_type
{
    AST_COMMAND,
    AST_COMMAND_LIST,
    AST_IF_CONDITION
};

struct ast
{
    enum ast_type type;
    char **command;
    struct ast *left;
    struct ast *right;
};

struct ast *ast_new(enum ast_type type);

void ast_free(struct ast *ast);

#endif /* !AST_H */
