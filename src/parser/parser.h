#ifndef PARSER_H
#define PARSER_H

#include "ast.h"
#include "lexer.h"

enum parser_status
{
    PARSER_OK,
    PARSER_ERROR
};

struct ast *parser_parse(struct lexer *lexer, enum parser_status *status);

#endif /* !PARSER_H */
