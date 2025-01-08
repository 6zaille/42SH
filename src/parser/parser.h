#ifndef PARSER_H
#define PARSER_H

#include "../lexer/token.h" // Inclut token.h depuis lexer
#include "ast.h"
#include "../lexer/lexer.h" // Inclut lexer.h depuis lexer

enum parser_status
{
    PARSER_OK,
    PARSER_ERROR,
};

struct ast *parse(enum parser_status *status, struct lexer *lexer);

struct ast *parser_parse(struct lexer *lexer, enum parser_status *status);


struct ast *parse_exp(enum parser_status *status, struct lexer *lexer);

struct ast *parse_sexp(enum parser_status *status, struct lexer *lexer);

struct ast *parse_texp(enum parser_status *status, struct lexer *lexer);

#endif /* !PARSER_H */
