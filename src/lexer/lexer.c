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

    const char *start = &lexer->input[lexer->pos];

    if (strncmp(start, "if", 2) == 0 && !isalnum(start[2]))
    {
        lexer->pos += 2;
        return create_token(TOKEN_IF, "if");
    }
    if (strncmp(start, "then", 4) == 0 && !isalnum(start[4]))
    {
        lexer->pos += 4;
        return create_token(TOKEN_THEN, "then");
    }
    if (strncmp(start, "else", 4) == 0 && !isalnum(start[4]))
    {
        lexer->pos += 4;
        return create_token(TOKEN_ELSE, "else");
    }
    if (strncmp(start, "elif", 4) == 0 && !isalnum(start[4]))
    {
        lexer->pos += 4;
        return create_token(TOKEN_ELIF, "elif");
    }
    if (strncmp(start, "fi", 2) == 0 && !isalnum(start[2]))
    {
        lexer->pos += 2;
        return create_token(TOKEN_FI, "fi");
    }

    if (lexer->input[lexer->pos] == ';')
    {
        lexer->pos++;
        return create_token(TOKEN_SEMICOLON, ";");
    }

    if (lexer->input[lexer->pos] == '\n')
    {
        lexer->pos++;
        return create_token(TOKEN_NEWLINE, "\\n");
    }

    size_t start_pos = lexer->pos;
    while (lexer->input[lexer->pos] && !isspace(lexer->input[lexer->pos]) &&
           lexer->input[lexer->pos] != ';' && lexer->input[lexer->pos] != '\n')
    {
        lexer->pos++;
    }
    size_t length = lexer->pos - start_pos;
    return create_token(TOKEN_WORD, strndup(&lexer->input[start_pos], length));
}

void token_free(struct token *tok)
{
    if (!tok)
        return;

    if (tok->value)
        free(tok->value);

    free(tok);
}