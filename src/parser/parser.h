#ifndef PARSER_H
#define PARSER_H

#include "../lexer/lexer.h"
#include "../lexer/token.h"

struct ast *parser_parse(struct lexer *lexer);
struct ast *parse_pipeline(struct lexer *lexer);

void ast_free(struct ast *node);

#endif /* !PARSER_H */
