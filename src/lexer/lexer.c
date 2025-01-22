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
    if (lexer->input[lexer->pos] == '#')
    {
        while (lexer->input[lexer->pos] && lexer->input[lexer->pos] != '\n')
        {
            lexer->pos++;
        }
    }
}

struct token *token_init(enum token_type type, char *value)
{
    struct token *tok = malloc(sizeof(struct token));
    tok->type = type;
    tok->value = value;
    return tok;
}

static struct token *handle_assignment(struct lexer *lexer)
{
    char *name = malloc(strlen(lexer->input) + 1);
    size_t name_index = 0;

    while (isalnum(lexer->input[lexer->pos]) || lexer->input[lexer->pos] == '_'
           || lexer->input[lexer->pos] == '$' || lexer->input[lexer->pos] == '{'
           || lexer->input[lexer->pos] == '}' || lexer->input[lexer->pos] == '@'
           || lexer->input[lexer->pos] == '*')
    {
        name[name_index++] = lexer->input[lexer->pos++];
    }
    name[name_index] = '\0';

    if (lexer->input[lexer->pos] == '=')
    {
        lexer->pos++; // Skip '='
        char *value = malloc(strlen(lexer->input) + 1);
        size_t value_index = 0;

        while (lexer->input[lexer->pos] && !isspace(lexer->input[lexer->pos])
               && lexer->input[lexer->pos] != ';')
        {
            value[value_index++] = lexer->input[lexer->pos++];
        }
        value[value_index] = '\0';

        // Utilise set_variable de utils.c
        set_variable(name, value);
        free(value);

        struct token *token = token_init(TOKEN_ASSIGNMENT, name);
        return token;
    }

    free(name);
    return NULL;
}

// Fonction pour analyser les substitutions
static struct token *handle_variable_substitution(struct lexer *lexer)
{
    lexer->pos++; // Skip '$'

    char *buffer = malloc(strlen(lexer->input) + 1);
    size_t buf_index = 0;

    while (isalnum(lexer->input[lexer->pos]) || lexer->input[lexer->pos] == '_'
           || lexer->input[lexer->pos] == '$' || lexer->input[lexer->pos] == '{'
           || lexer->input[lexer->pos] == '}' || lexer->input[lexer->pos] == '@'
           || lexer->input[lexer->pos] == '*' || lexer->input[lexer->pos] == '?'
           || lexer->input[lexer->pos] == '#')
    {
        buffer[buf_index++] = lexer->input[lexer->pos++];
    }
    buffer[buf_index] = '\0';

    // Récupère la valeur via get_variable de utils.c
    const char *value = get_variable(buffer);
    free(buffer);

    return token_init(TOKEN_WORD, strdup(value ? value : ""));
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
    if (strcmp(word, "while") == 0)
        return TOKEN_WHILE;
    if (strcmp(word, "until") == 0)
        return TOKEN_UNTIL;
    if (strcmp(word, "do") == 0)
        return TOKEN_DO;
    if (strcmp(word, "done") == 0)
        return TOKEN_DONE;

    return TOKEN_WORD;
}
/*
static int is_surrounded_by_letters(const char *input, size_t pos)
{
    char prev = (pos > 0) ? input[pos - 1] : ' ';
    char next = input[pos + 1];
    return isalnum(prev) && isalnum(next);
}*/

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
    lexer->current_tok = NULL;
    return lexer;
}

static struct token *handle_word_token(struct lexer *lexer)
{
    char *buffer = malloc(strlen(lexer->input) + 1);
    size_t buf_index = 0;

    while (lexer->input[lexer->pos] && !isspace(lexer->input[lexer->pos])
           && lexer->input[lexer->pos] != ';'
           && lexer->input[lexer->pos] != '\n')
    {
        if (lexer->input[lexer->pos] == '\\') // Handle escape sequences
        {
            lexer->pos++; // Skip escape character
            buffer[buf_index++] = lexer->input[lexer->pos++];
        }
        else if (lexer->input[lexer->pos] == '"') // Handle double quotes
        {
            lexer->pos++; // Skip opening quote
            while (lexer->input[lexer->pos] && lexer->input[lexer->pos] != '"')
            {
                if (lexer->input[lexer->pos] == '\\'
                    && (lexer->input[lexer->pos + 1] == '$'
                        || lexer->input[lexer->pos + 1] == '"'))
                {
                    lexer->pos++; // Skip escape character
                }
                else if (lexer->input[lexer->pos]
                         == '$') // Handle variable substitution
                {
                    lexer->pos++;
                    char var_name[256] = { 0 };
                    size_t var_index = 0;

                    // Extract the variable name
                    while (isalnum(lexer->input[lexer->pos])
                           || lexer->input[lexer->pos] == '_')
                    {
                        var_name[var_index++] = lexer->input[lexer->pos++];
                    }
                    var_name[var_index] = '\0';

                    // Fetch the variable value
                    const char *value = get_variable(var_name);
                    if (value)
                    {
                        strcpy(&buffer[buf_index], value);
                        buf_index += strlen(value);
                    }
                }
                else
                {
                    buffer[buf_index++] = lexer->input[lexer->pos++];
                }
            }
            if (lexer->input[lexer->pos] == '"')
            {
                lexer->pos++; // Skip closing quote
            }
        }
        else if (lexer->input[lexer->pos]
                 == '$') // Handle variable substitution outside quotes
        {
            lexer->pos++;
            char var_name[256] = { 0 };
            size_t var_index = 0;

            while (isalnum(lexer->input[lexer->pos])
                   || lexer->input[lexer->pos] == '_')
            {
                var_name[var_index++] = lexer->input[lexer->pos++];
            }
            var_name[var_index] = '\0';

            const char *value = get_variable(var_name);
            if (value)
            {
                strcpy(&buffer[buf_index], value);
                buf_index += strlen(value);
            }
        }
        else
        {
            buffer[buf_index++] = lexer->input[lexer->pos++];
        }
    }

    buffer[buf_index] = '\0';
    enum token_type type = check_keyword(buffer);
    return token_init(type, buffer);
}

struct token *lexer_next_token(struct lexer *lexer)
{
    skip_whitespace(lexer);
    skip_comment(lexer);

    if (lexer->input[lexer->pos] == '\0')
    {
        return token_init(TOKEN_EOF, NULL);
    }

    char c = lexer->input[lexer->pos];

    if (c == '&' && lexer->input[lexer->pos + 1] == '&')
    {
        lexer->pos += 2;
        return token_init(TOKEN_AND, strdup("&&"));
    }
    if (c == '|')
    {
        if (lexer->input[lexer->pos + 1] == '|')
        {
            lexer->pos += 2;
            return token_init(TOKEN_OR, strdup("||"));
        }
        else
        {
            lexer->pos++;
            return token_init(TOKEN_PIPE, strdup("|"));
        }
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
            return token_init(TOKEN_NEWLINE, NULL);
        }
    }
    else if (c == '$')
    {
        return handle_variable_substitution(lexer);
    }
    else if (c == '\n')
    {
        lexer->pos++;
        return token_init(TOKEN_NEWLINE, NULL);
    }
    else if (isalnum(c) || c == '_')
    {
        size_t temp_pos = lexer->pos;
        while (isalnum(lexer->input[temp_pos]) || lexer->input[temp_pos] == '_')
        {
            temp_pos++;
        }
        if (lexer->input[temp_pos] == '=')
        {
            return handle_assignment(lexer);
        }
        return handle_word_token(lexer);
    }
    else if (c == ';')
    {
        lexer->pos++;
        return token_init(TOKEN_SEMICOLON, strdup(";"));
    }
    else if (c == '!')
    {
        lexer->pos++;
        return token_init(TOKEN_NEGATION, strdup("!"));
    }
    else
    {
        return handle_word_token(lexer);
    }

    return token_init(TOKEN_ERROR, strdup("char error"));
}

void token_free(struct token *tok)
{
    free(tok->value);
    free(tok);
}

struct token lexer_peek(struct lexer *lexer)
{
    if (lexer->current_tok)
    {
        return *lexer->current_tok;
    }

    struct token *next_tok = lexer_next_token(lexer);
    lexer->current_tok = next_tok;
    return *next_tok;
}

struct token lexer_pop(struct lexer *lexer)
{
    if (lexer->current_tok)
    {
        token_free(lexer->current_tok);
    }
    struct token *next_tok = lexer_next_token(lexer);
    lexer->current_tok = next_tok;
    return *next_tok;
}

void print_variable(void)
{
    for (size_t i = 0; i < variable_count; i++)
    {
        printf("Variable name: %s, Variable value: %s\n", variables[i].name,
               variables[i].value);
    }
}

void lexer_destroy(struct lexer *lexer)
{
    for (size_t i = 0; i < variable_count; i++)
    {
        free(variables[i].name);
        free(variables[i].value);
    }
    token_free(lexer->current_tok);
    free(lexer);
}
