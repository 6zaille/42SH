#define _POSIX_C_SOURCE 200809L
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "lexer.h"
#include "token.h"

static struct token *create_token(enum token_type type, const char *value)
{
    struct token *tok = malloc(sizeof(struct token));
    if (!tok)
        return NULL;

    tok->type = type;
    if (value)
    {
        tok->value = strdup(value);
        if (!tok->value)
        {
            free(tok);
            return NULL;
        }
    }
    else
    {
        tok->value = NULL;
    }
    return tok;
}

static void skip_whitespace(struct lexer *lexer)
{
    while (lexer->input[lexer->pos] == ' ' || lexer->input[lexer->pos] == '\t')
        lexer->pos++;
}

static struct token *read_word(struct lexer *lexer)
{
    size_t start = lexer->pos;
    while (lexer->input[lexer->pos] && lexer->input[lexer->pos] != ' ' &&
           lexer->input[lexer->pos] != '\t' && lexer->input[lexer->pos] != '\n' &&
           lexer->input[lexer->pos] != ';')
    {
        lexer->pos++;
    }
    size_t length = lexer->pos - start;
    char *word = strndup(&lexer->input[start], length);
    if (!word)
        return NULL;
    return create_token(TOKEN_WORD, word);
}

struct lexer *lexer_init(const char *input)
{
    struct lexer *lexer = malloc(sizeof(struct lexer));
    if (!lexer)
        return NULL;
    lexer->input = input;
    lexer->pos = 0;
    return lexer;
}

void lexer_destroy(struct lexer *lexer)
{
    if (lexer)
        free(lexer);
}

struct token *lexer_next_token(struct lexer *lexer)
{
    skip_whitespace(lexer);

    if (lexer->input[lexer->pos] == '\0')
        return create_token(TOKEN_EOF, NULL);

    char current = lexer->input[lexer->pos];
    lexer->pos++;

    switch (current)
    {
    case ';':
        return create_token(TOKEN_SEMICOLON, ";");
    case '\n':
        return create_token(TOKEN_NEWLINE, "\\n");
    case '\'':
    {
        size_t start = lexer->pos;
        while (lexer->input[lexer->pos] && lexer->input[lexer->pos] != '\'')
            lexer->pos++;

        if (lexer->input[lexer->pos] == '\'')
        {
            size_t length = lexer->pos - start;
            char *quoted_content = strndup(&lexer->input[start], length);
            lexer->pos++; // Skip closing quote
            return create_token(TOKEN_WORD, quoted_content);
        }
        else
        {
            // Unterminated single quote
            return create_token(TOKEN_ERROR, "Unterminated single quote");
        }
    }
    }

    // Default case: Read a word
    lexer->pos--;
    return read_word(lexer);
}

void token_free(struct token *tok)
{
    if (!tok)
        return;

    if (tok->value)
        free(tok->value);

    free(tok);
}
