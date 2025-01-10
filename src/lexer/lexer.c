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
    skip_whitespace(lexer); // Skip any whitespace characters

    if (lexer->input[lexer->pos] == '\0') // Check for end of input
    {
        return create_token(TOKEN_EOF, NULL); // Return end of file token
    }

    char c = lexer->input[lexer->pos]; // Get the current character
    if (c == '\'') // Handle single quote
    {
        if (is_surrounded_by_letters(lexer, lexer->pos)) // Check if single quote is surrounded by letters
        {
            lexer->pos++; // Ignore this single quote
            return lexer_next_token(lexer); // Continue to the next token
        }
        return create_token(TOKEN_SINGLE_QUOTE, NULL); // Return single quote token
    }
    else if (c == '\\') // Handle backslash
    {
        lexer->pos++;
        if (lexer->input[lexer->pos] == 'n') // Check for newline escape sequence
        {
            lexer->pos++;
            return create_token(TOKEN_NEWLINE, NULL); // Return newline token
        }
    }
    else if (c == '\n') // Handle newline
    {
        lexer->pos++;
        return create_token(TOKEN_NEWLINE, NULL); // Return newline token
    }
    else if (c == ';') // Handle semicolon
    {
        lexer->pos++;
        return create_token(TOKEN_SEMICOLON, NULL); // Return semicolon token
    }
    else // Handle other characters
    {
        size_t start = lexer->pos; // Mark the start of the token
        while (lexer->input[lexer->pos] && !isspace(lexer->input[lexer->pos])
               && lexer->input[lexer->pos] != ';'
               && lexer->input[lexer->pos] != '\''
               && lexer->input[lexer->pos] != '\\'
               && lexer->input[lexer->pos] != '\n') // Continue until a delimiter is found
        {
            lexer->pos++;
        }

        size_t len = lexer->pos - start; // Calculate the length of the token
        char *value = strndup(lexer->input + start, len); // Duplicate the token string
        enum token_type type = check_keyword(value); // Check if the token is a keyword
        struct token *tok;

        if (type != TOKEN_WORD) // If it's a keyword
        {
            tok = create_token(type, NULL); // Create a keyword token
        }
        else // If it's a regular word
        {
            tok = create_token(TOKEN_WORD, value); // Create a word token
        }

        free(value); // Free the duplicated string
        return tok; // Return the created token
    }

    return create_token(TOKEN_ERROR, "Unexpected character"); // Return an error token for unexpected characters
}

void token_free(struct token *token)
{
    if (token)
    {
        free(token->value);
        free(token);
    }
}
