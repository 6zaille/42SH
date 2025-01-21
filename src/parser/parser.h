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
    AST_PIPELINE,
    AST_NEGATION,
    AST_WHILE,
    AST_UNTIL,

};
enum parser_status
{
    PARSER_OK, // Tout va bien
    PARSER_UNEXPECTED_TOKEN, // Token inattendu rencontré
    PARSER_ERROR, // Autres erreurs
};

struct ast *ast_create(enum ast_type type);
struct ast *parser_parse(struct lexer *lexer);
struct ast *parse_if_statement(struct lexer *lexer);
struct ast *parse_pipeline(struct lexer *lexer);
struct ast *parse_command_list(struct lexer *lexer);
struct ast *parse_rule_while(enum parser_status *status, struct lexer *lexer);
void ast_free(struct ast *node);

#endif /* !PARSER_H */
