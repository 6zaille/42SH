#ifndef LEXER_H
#define LEXER_H

#include "../utils/utils.h"
#include "token.h"

struct lexer
{
    const char *input;
    size_t pos;
    struct token *current_tok;
};

struct lexer *lexer_init(const char *input);

void lexer_destroy(struct lexer *lexer);

struct token *lexer_next_token(struct lexer *lexer);

enum token_type check_keyword(const char *word);

struct token lexer_peek(struct lexer *lexer);

struct token lexer_pop(struct lexer *lexer);

void print_variable(void);

#endif /* !LEXER_H */
