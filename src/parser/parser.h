#ifndef PARSER_H
#define PARSER_H

#include "../lexer/lexer.h"
#include "../lexer/token.h"

extern int last_exit_status;

enum ast_type
{
    AST_COMMAND,
    AST_SIMPLE_COMMAND,
    AST_LIST,
    AST_IF,
    AST_PIPELINE,
    AST_NEGATION,
    AST_WHILE,
    AST_UNTIL,
    AST_AND_OR
};

struct ast *ast_create(enum ast_type type);
struct ast *parser_parse(struct lexer *lexer);
struct ast *parse_if_statement(struct lexer *lexer);
struct ast *parse_pipeline(struct lexer *lexer);
struct ast *parse_command_list(struct lexer *lexer);
struct ast *parse_and_or(struct lexer *lexer);
struct ast *parse_while(struct lexer *lexer);
void ast_free(struct ast *node);

#endif /* !PARSER_H */
