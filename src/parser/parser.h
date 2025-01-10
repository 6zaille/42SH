#ifndef PARSER_H
#define PARSER_H

#include "../lexer/lexer.h"
#include "../lexer/token.h"
#include "ast.h"

enum parser_status
{
    PARSER_OK,
    PARSER_ERROR,
};

struct ast *parse(enum parser_status *status, struct lexer *lexer);

struct ast *parser_parse(struct lexer *lexer, enum parser_status *status);

#endif /* !PARSER_H */
