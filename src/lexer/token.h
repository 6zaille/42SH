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
    TOKEN_ERROR,
    TOKEN_OK,
    TOKEN_PIPE,
    TOKEN_REDIRECT_IN, // <
    TOKEN_REDIRECT_OUT, // >
    TOKEN_REDIRECT_APPEND, // >>
    TOKEN_REDIRECT_DUP_OUT, // >&
    TOKEN_REDIRECT_DUP_IN, // <&
    TOKEN_REDIRECT_CLOBBER, // >|
    TOKEN_REDIRECT_RW, // <>
    TOKEN_NEGATION,
    TOKEN_VARIABLE,
    TOKEN_ASSIGNMENT,
    TOKEN_WHILE,
    TOKEN_DO,
    TOKEN_UNTIL,
    TOKEN_DONE,
    TOKEN_AND,
    TOKEN_OR,
    TOKEN_IN,
    TOKEN_FOR,
};

struct token
{
    enum token_type type;
    char *value;
};

void token_free(struct token *tok);

#endif /* !TOKEN_H */
