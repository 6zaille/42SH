#define _POSIX_C_SOURCE 200809L
#include "lexer.h"

#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "token.h"

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

enum token_type check_keyword(const char *word)
{
    if (strcmp(word, "if") == 0)
        return TOKEN_IF;
    if (strcmp(word, "then") == 0)
        return TOKEN_THEN;
    if (strcmp(word, "elif") == 0)
        return TOKEN_ELIF;
    if (strcmp(word, "else") == 0)
        return TOKEN_ELSE;
    if (strcmp(word, "fi") == 0)
        return TOKEN_FI;

    return TOKEN_WORD;
}

static int is_surrounded_by_letters(const char *input, size_t pos)
{
    char prev = (pos > 0) ? input[pos - 1] : ' ';
    char next = input[pos + 1];
    return isalnum(prev) && isalnum(next);
}

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
    lexer->pushed_back_token = NULL;
    return lexer;
}

void lexer_push_back(struct lexer *lexer, struct token *token)
{
    if (lexer->pushed_back_token)
    {
        token_free(lexer->pushed_back_token);
    }
    lexer->pushed_back_token = token;
}

struct token *lexer_next_token(struct lexer *lexer)
{
    if (lexer->pushed_back_token)
    {
        struct token *tok = lexer->pushed_back_token;
        lexer->pushed_back_token = NULL;
        return tok;
    }

    skip_whitespace(lexer);

    if (lexer->input[lexer->pos] == '\0') // Fin de l'entrée
    {
        return create_token(TOKEN_EOF, NULL);
    }

    char c = lexer->input[lexer->pos];
    if (c == '\'') // Gestion des single quotes
    {
        lexer->pos++; // Skip the quote
        return lexer_next_token(lexer);
    }
    else if (c == '\\') // Gestion des backslashes suivis de 'n'
    {
        lexer->pos++;
        if (lexer->input[lexer->pos] == 'n')
        {
            lexer->pos++;
            return create_token(TOKEN_NEWLINE, NULL);
        }
    }
    else if (c == '\n') // Ignore les newlines
    {
        lexer->pos++;
        return lexer_next_token(lexer);
    }
    else if (c == ';') // Gestion des points-virgules
    {
        lexer->pos++;
        return create_token(TOKEN_SEMICOLON, NULL);
    }
    else // Gestion des mots
    {
        char *buffer = malloc(strlen(lexer->input) + 1);
        size_t buf_index = 0;

        while (lexer->input[lexer->pos] && !isspace(lexer->input[lexer->pos])
               && lexer->input[lexer->pos] != ';'
               && lexer->input[lexer->pos] != '\\'
               && lexer->input[lexer->pos] != '\n')
        {
            if (lexer->input[lexer->pos] == '\''
                && is_surrounded_by_letters(lexer->input, lexer->pos))
            {
                lexer->pos++; // Skip the quote
                continue;
            }
            buffer[buf_index++] = lexer->input[lexer->pos++];
        }

        buffer[buf_index] = '\0';
        enum token_type type = check_keyword(buffer);
        struct token *tok;

        if (type != TOKEN_WORD)
        {
            tok = create_token(type, NULL);
        }
        else
        {
            tok = create_token(TOKEN_WORD, buffer);
        }

        free(buffer);
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

void lexer_destroy(struct lexer *lexer)
{
    if (lexer->pushed_back_token)
    {
        token_free(lexer->pushed_back_token);
    }
    free(lexer);
}
