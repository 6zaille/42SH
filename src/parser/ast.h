#ifndef AST_H
#define AST_H

#include <err.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

#include "../execution/builtins.h"
#include "../execution/exec.h"
#include "../lexer/lexer.h"
#include "../lexer/token.h"
#include "ast.h"
#include "parser.h"

void ast_eval(struct ast *node);
void ast_pretty_print(struct ast *node, int depth);

enum redirection_type
{
    REDIR_OUT, // >
    REDIR_IN, // <
    REDIR_APPEND, // >>
    REDIR_DUP_OUT, // >&
    REDIR_DUP_IN, // <&
    REDIR_CLOBBER, // >|
    REDIR_RW // <>
};

enum ast_type
{
    AST_COMMAND,
    AST_SIMPLE_COMMAND,
    AST_LIST,
    AST_IF,
    AST_PIPELINE,
    AST_NEGATION
};

struct ast_if_data
{
    struct ast *condition;
    struct ast *then_branch;
    struct ast *else_branch;
};

struct ast
{
    enum ast_type type;
    size_t children_count;
    struct ast **children;
    void *data;
};

struct ast_command_data
{
    char **args;
    struct redirection *redirections; // tableau de redirections
    size_t redirection_count; // nombre de redirections
};

extern int last_exit_status;

struct redirection
{
    enum redirection_type type;
    char *filename;
};

#endif /* !AST_H */
