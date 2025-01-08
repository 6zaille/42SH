#ifndef TOKEN_H
#define TOKEN_H

#include <unistd.h>

enum token_type
{
    TOKEN_IF,
    TOKEN_THEN,
    TOKEN_ELIF,
    TOKEN_ELSE,
    TOKEN_FI,
    TOKEN_SEMICOLON,
    TOKEN_NEWLINE,
    TOKEN_WORD,
    TOKEN_SINGLE_QUOTE,
    TOKEN_EOF,
    TOKEN_ERROR
};

struct token
{
    enum token_type type;
    char *value;
};

#endif /* !TOKEN_H */
