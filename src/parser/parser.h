#ifndef PARSER_H
#define PARSER_H

#include "../lexer/lexer.h"
#include "../lexer/token.h"

enum ast_type
{
    AST_COMMAND,
    AST_SIMPLE_COMMAND,
    AST_LIST,
    AST_IF,
    AST_PIPELINE
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

struct ast_command_data {
    char **args;
    struct redirection *redirections;
    size_t redirection_count;
};

struct ast *parser_parse(struct lexer *lexer);
struct ast *parse_pipeline(struct lexer *lexer);

void ast_free(struct ast *node);

#endif /* !PARSER_H */
