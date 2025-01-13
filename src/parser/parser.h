#ifndef PARSER_H
#define PARSER_H

#include "../lexer/lexer.h"
#include "../lexer/token.h"

enum ast_type
{
    AST_COMMAND,
    AST_SIMPLE_COMMAND,
    AST_LIST,
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
};

struct ast *parser_parse(struct lexer *lexer);

void ast_free(struct ast *node);

#endif /* !PARSER_H */
