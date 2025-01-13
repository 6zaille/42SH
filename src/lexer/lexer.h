#ifndef LEXER_H
#define LEXER_H

#include "token.h"

struct lexer
{
    const char *input;
    size_t pos;
    struct token *pushed_back_token;
};

struct lexer *lexer_init(const char *input);

void lexer_destroy(struct lexer *lexer);

struct token *lexer_next_token(struct lexer *lexer);

void token_free(struct token *token);

enum token_type check_keyword(const char *word);

void lexer_push_back(struct lexer *lexer, struct token *token);

#endif /* !LEXER_H */
