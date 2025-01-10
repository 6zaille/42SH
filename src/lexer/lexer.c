#define _POSIX_C_SOURCE 200809L
#include "lexer.h"

#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "token.h"

static const struct
{
    const char *keyword;
    enum token_type type;
} keywords[] = {
    { "if", TOKEN_IF },     { "then", TOKEN_THEN }, { "elif", TOKEN_ELIF },
    { "else", TOKEN_ELSE }, { "fi", TOKEN_FI },
};

static struct token *create_token(enum token_type type, const char *value)
{
    struct token *tok = malloc(sizeof(struct token));
    if (!tok)
    {
        perror("malloc");
        return NULL;
    }
    tok->type = type;
    tok->value = value ? strdup(value) : NULL;
    return tok;
}

static void skip_whitespace(struct lexer *lexer)
{
    while (lexer->input[lexer->pos] == ' ' || lexer->input[lexer->pos] == '\t')
    {
        lexer->pos++;
    }
}

static enum token_type check_keyword(const char *word)
{
    for (size_t i = 0; i < sizeof(keywords) / sizeof(keywords[0]); i++)
    {
        if (strcmp(word, keywords[i].keyword) == 0)
        {
            return keywords[i].type;
        }
    }
    return TOKEN_WORD;
}

static int is_surrounded_by_letters(struct lexer *lexer, size_t pos)
{
    char prev = (pos > 0) ? lexer->input[pos - 1] : ' ';
    char next = lexer->input[pos + 1];
    return isalnum(prev) && isalnum(next);
}

/*
static struct token *handle_single_quote(struct lexer *lexer) {
    size_t start = ++lexer->pos; // Skip opening single quote
    while (lexer->input[lexer->pos] && lexer->input[lexer->pos] != '\'') {
        lexer->pos++;
    }

    if (lexer->input[lexer->pos] == '\0') {
        return create_token(TOKEN_ERROR, "Unmatched single quote");
    }

    size_t len = lexer->pos - start;
    char *value = strndup(lexer->input + start, len);
    lexer->pos++; // Skip closing single quote
    return create_token(TOKEN_WORD, value);
}
*/
struct lexer *lexer_init(const char *input)
{
    struct lexer *lexer = malloc(sizeof(struct lexer));
    if (!lexer)
    {
        perror("malloc");
        return NULL;
    }
    lexer->input = input;
    lexer->pos = 0;
    return lexer;
}

void lexer_destroy(struct lexer *lexer)
{
    free(lexer);
}

struct token *lexer_next_token(struct lexer *lexer)
{
    skip_whitespace(lexer);

    if (lexer->input[lexer->pos] == '\0')
    {
        return create_token(TOKEN_EOF, NULL);
    }

    char c = lexer->input[lexer->pos];
    if (c == '\'')
    {
        if (is_surrounded_by_letters(lexer, lexer->pos))
        {
            lexer->pos++; // Ignore this single quote
            return lexer_next_token(lexer);
        }
        return create_token(TOKEN_SINGLE_QUOTE, NULL);
    }
    else if (c == '\\')
    {
        lexer->pos++;
        if (lexer->input[lexer->pos] == 'n')
        {
            lexer->pos++;
            return create_token(TOKEN_NEWLINE, NULL);
        }
    }
    else if (c == '\n')
    {
        lexer->pos++;
        return create_token(TOKEN_NEWLINE, NULL);
    }
    else if (c == ';')
    {
        lexer->pos++;
        return create_token(TOKEN_SEMICOLON, NULL);
    }
    else
    {
        size_t start = lexer->pos;
        while (lexer->input[lexer->pos] && !isspace(lexer->input[lexer->pos])
               && lexer->input[lexer->pos] != ';'
               && lexer->input[lexer->pos] != '\''
               && lexer->input[lexer->pos] != '\\'
               && lexer->input[lexer->pos] != '\n')
        {
            lexer->pos++;
        }

        size_t len = lexer->pos - start;
        char *value = strndup(lexer->input + start, len);
        enum token_type type = check_keyword(value);
        struct token *tok;

        if (type != TOKEN_WORD)
        {
            tok = create_token(type, NULL);
        }
        else
        {
            tok = create_token(TOKEN_WORD, value);
        }

        free(value);
        return tok;
    }

    return create_token(TOKEN_ERROR, "Unexpected character");
}

void token_free(struct token *token)
{
    if (token)
    {
        free(token->value);
        free(token);
    }
}
