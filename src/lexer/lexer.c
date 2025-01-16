#define _POSIX_C_SOURCE 200809L

#include "lexer.h"

#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "token.h"

static void skip_whitespace(struct lexer *lexer)
{
    while (lexer->input[lexer->pos] == ' ' || lexer->input[lexer->pos] == '\t')
    {
        lexer->pos++;
    }
}

static void skip_comment(struct lexer *lexer)
{
    if (lexer->input[lexer->pos] == '#') // Si on rencontre un commentaire
    {
        while (lexer->input[lexer->pos]
               && lexer->input[lexer->pos]
                   != '\n') // On avance jusqu'à la fin de la ligne
        {
            lexer->pos++;
        }
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
    lexer->current_tok.type = TOKEN_EOF;
    lexer->current_tok.value = NULL;
    return lexer;
}

static struct token handle_word_token(struct lexer *lexer)
{
    char *buffer = malloc(strlen(lexer->input) + 1);
    size_t buf_index = 0;

    while (lexer->input[lexer->pos] && !isspace(lexer->input[lexer->pos])
           && lexer->input[lexer->pos] != ';'
           && lexer->input[lexer->pos] != '\n')
    {
        if (lexer->input[lexer->pos] == '\''
            && is_surrounded_by_letters(lexer->input, lexer->pos))
        {
            lexer->pos++;
            continue;
        }
        buffer[buf_index++] = lexer->input[lexer->pos++];
    }

    buffer[buf_index] = '\0';
    enum token_type type = check_keyword(buffer);
    struct token tok = { type, buffer };
    return tok;
}

struct token lexer_next_token(struct lexer *lexer)
{
    skip_whitespace(lexer);
    skip_comment(lexer);

    if (lexer->input[lexer->pos] == '\0')
    {
        return (struct token){ TOKEN_EOF, NULL };
    }

    char c = lexer->input[lexer->pos];

    if (c == '|')
    {
        lexer->pos++;
        return (struct token){ TOKEN_PIPE, strdup("|") };
    }
    else if (c == '\'')
    {
        lexer->pos++;
        return lexer_next_token(lexer);
    }
    else if (c == '\\')
    {
        lexer->pos++;
        if (lexer->input[lexer->pos] == 'n')
        {
            lexer->pos++;
            return (struct token){ TOKEN_NEWLINE, NULL };
        }
    }
    else if (c == '\n')
    {
        lexer->pos++;
        return (struct token){ TOKEN_NEWLINE, NULL };
    }
    else if (c == ';')
    {
        lexer->pos++;
        return (struct token){ TOKEN_SEMICOLON, strdup(";") };
    }
    else
    {
        return handle_word_token(lexer);
    }

    return (struct token){ TOKEN_ERROR, strdup("char error") };
}

struct token lexer_peek(struct lexer *lexer)
{
    size_t saved_pos = lexer->pos;
    struct token next_tok = lexer_next_token(lexer);
    lexer->pos = saved_pos;
    return next_tok;
}

struct token lexer_pop(struct lexer *lexer)
{
    struct token next_tok = lexer_next_token(lexer);
    lexer->current_tok = next_tok;
    return next_tok;
}

void lexer_destroy(struct lexer *lexer)
{
    free(lexer->current_tok.value);
    free(lexer);
}
