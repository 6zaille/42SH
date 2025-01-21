#define _POSIX_C_SOURCE 200809L

#include "parser.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "../lexer/lexer.h"
#include "../lexer/token.h"
#include "ast.h"

struct ast *ast_create(enum ast_type type)
{
    struct ast *node = malloc(sizeof(*node));
    if (!node)
    {
        return NULL;
    }
    node->type = type;
    node->children_count = 0;
    node->children = NULL;
    node->data = NULL;
    return node;
}

static void *safe_realloc(void *ptr, size_t new_size)
{
    void *new_ptr = realloc(ptr, new_size);
    if (!new_ptr)
        free(ptr);
    return new_ptr;
}

static char **append_arg(char **args, const char *arg)
{
    size_t count = 0;
    while (args && args[count])
        count++;
    char **new_args = safe_realloc(args, (count + 2) * sizeof(char *));
    if (!new_args)
        return NULL;
    new_args[count] = strdup(arg);
    new_args[count + 1] = NULL;
    return new_args;
}

static int is_special_token(enum token_type type)
{
    return type == TOKEN_IF || type == TOKEN_THEN || type == TOKEN_ELSE
        || type == TOKEN_ELIF || type == TOKEN_FI || type == TOKEN_DONE
        || type == TOKEN_WHILE || type == TOKEN_DO || type == TOKEN_UNTIL;
}

static void convert_token_to_word(struct token *tok)
{
    if (is_special_token(tok->type))
    {
        tok->type = TOKEN_WORD;
    }
}

static struct redirection *parse_redirection(struct lexer *lexer)
{
    struct token tok = lexer_peek(lexer);
    if (tok.type != TOKEN_REDIRECT_IN && tok.type != TOKEN_REDIRECT_OUT)
    {
        return NULL;
    }
    lexer_pop(lexer);

    struct token filename = lexer_peek(lexer);
    if (filename.type != TOKEN_WORD)
    {
        fprintf(stderr, "erreur filename redirection\n");
        return NULL;
    }
    lexer_pop(lexer);

    struct redirection *redir = malloc(sizeof(*redir));
    redir->type = (tok.type == TOKEN_REDIRECT_IN) ? REDIR_IN : REDIR_OUT;
    redir->filename = strdup(filename.value);
    return redir;
}

static struct ast *create_assignment_node(const char *value)
{
    struct ast *assignment_node = ast_create(AST_SIMPLE_COMMAND);
    if (!assignment_node)
    {
        printf("Failed to create assignment node\n");
        return NULL;
    }

    struct ast_command_data *assignment_data = malloc(sizeof(*assignment_data));
    if (!assignment_data)
    {
        printf("Failed to allocate memory for assignment_data\n");
        free(assignment_node);
        return NULL;
    }

    assignment_data->args = append_arg(NULL, value);
    assignment_node->data = assignment_data;

    return assignment_node;
}

static int add_assignment_node(struct ast *cmd_node, struct lexer *lexer)
{
    struct token tok = lexer_peek(lexer);
    struct ast *assignment_node = create_assignment_node(tok.value);
    if (!assignment_node)
    {
        return -1;
    }

    cmd_node->children =
        realloc(cmd_node->children,
                sizeof(struct ast *) * (cmd_node->children_count + 1));
    if (!cmd_node->children)
    {
        perror("realloc");
        ast_free(assignment_node);
        return -1;
    }

    cmd_node->children[cmd_node->children_count++] = assignment_node;
    lexer_pop(lexer);
    return 0;
}

static int add_argument_or_redirection(struct ast_command_data *data,
                                       struct lexer *lexer, struct token tok)
{
    if (tok.type == TOKEN_VARIABLE)
    {
        const char *value = get_variable(tok.value);
        data->args = append_arg(data->args, value);
    }
    else if (tok.type == TOKEN_WORD)
    {
        if (strcmp(tok.value,"")==0)
        {
            return 0;
        }
        data->args = append_arg(data->args, tok.value);
    }
    else
    {
        struct redirection *redir = parse_redirection(lexer);
        if (redir)
        {
            data->redirections = realloc(data->redirections,
                                         sizeof(*data->redirections)
                                             * (data->redirection_count + 1));
            if (!data->redirections)
            {
                perror("realloc");
                free(redir);
                return -1;
            }
            data->redirections[data->redirection_count++] = *redir;
            free(redir);
        }
    }
    return 0;
}

static struct ast *parse_simple_command(struct lexer *lexer)
{
    struct ast *cmd_node = ast_create(AST_SIMPLE_COMMAND);
    if (!cmd_node)
    {
        return NULL;
    }

    struct ast_command_data *data = malloc(sizeof(*data));
    if (!data)
    {
        free(cmd_node);
        return NULL;
    }
    data->args = NULL;
    data->redirections = NULL;
    data->redirection_count = 0;

    struct token tok = lexer_peek(lexer);
    while (tok.type == TOKEN_WORD || tok.type == TOKEN_VARIABLE
           || tok.type == TOKEN_ASSIGNMENT
           || (tok.type >= TOKEN_REDIRECT_IN && tok.type <= TOKEN_REDIRECT_RW)
           || is_special_token(tok.type))
    {
        convert_token_to_word(&tok);
        if (tok.type == TOKEN_ASSIGNMENT)
        {
            if (add_assignment_node(cmd_node, lexer) != 0)
            {
                ast_free(cmd_node);
                return NULL;
            }
        }
        else
        {
            if (add_argument_or_redirection(data, lexer, tok) != 0)
            {
                ast_free(cmd_node);
                return NULL;
            }
        }
        tok = lexer_pop(lexer);
    }

    cmd_node->data = data;
    return cmd_node;
}


static struct ast *parse_change(struct lexer *lexer)
{
    // printf("Parsing change...\n");
    struct token tok = lexer_peek(lexer);

    if (tok.type == TOKEN_NEGATION)
    {
        lexer_pop(lexer);
        struct ast *child = parse_change(lexer);
        if (!child)
        {
            fprintf(stderr, "Failed to parse negation child\n");
            return NULL;
        }

        struct ast *negation_node = ast_create(AST_NEGATION);
        if (!negation_node)
        {
            ast_free(child);
            return NULL;
        }

        negation_node->children = malloc(sizeof(struct ast *));
        if (!negation_node->children)
        {
            ast_free(negation_node);
            ast_free(child);
            return NULL;
        }

        negation_node->children[0] = child;
        negation_node->children_count = 1;
        return negation_node;
    }

    // printf("Calling parse_pipeline...\n");
    return parse_pipeline(lexer);
}

static int is_end_of_block(enum token_type type)
{
    return type == TOKEN_DO || type == TOKEN_DONE || type == TOKEN_THEN
        || type == TOKEN_ELSE || type == TOKEN_ELIF || type == TOKEN_FI
        || type == TOKEN_EOF;
}

struct ast *parse_command_list(struct lexer *lexer)
{
    struct ast *list_node = ast_create(AST_LIST);
    if (!list_node)
        return NULL;

    struct ast **commands = NULL;
    size_t count = 0;

    while (1)
    {
        struct token tok = lexer_peek(lexer);

        // Vérifier si nous sommes à la fin d'un bloc
        if (is_end_of_block(tok.type))
        {
            break;
        }

        // Appeler parse_and_or pour chaque commande
        struct ast *command_node = parse_and_or(lexer);
        if (!command_node)
        {
            ast_free(list_node);
            return NULL;
        }

        commands = realloc(commands, sizeof(struct ast *) * (count + 1));
        if (!commands)
        {
            ast_free(command_node);
            ast_free(list_node);
            return NULL;
        }

        commands[count++] = command_node;

        // Gérer les séparateurs comme ';' et '\n'
        tok = lexer_peek(lexer);
        if (tok.type == TOKEN_SEMICOLON || tok.type == TOKEN_NEWLINE)
        {
            lexer_pop(lexer);
        }
        else if (tok.type == TOKEN_EOF)
        {
            break;
        }
        else
        {
            // Token inattendu
            ast_free(list_node);
            return NULL;
        }
    }

    list_node->children = commands;
    list_node->children_count = count;
    return list_node;
}

struct ast *parse_while(struct lexer *lexer)
{
    struct token tok = lexer_peek(lexer);

    if (tok.type != TOKEN_WHILE && tok.type != TOKEN_UNTIL)
    {
        return NULL; // Erreur : pas un TOKEN_WHILE ou TOKEN_UNTIL
    }
    lexer_pop(lexer);

    struct ast *ast_loop =
        ast_create(tok.type == TOKEN_WHILE ? AST_WHILE : AST_UNTIL);
    if (!ast_loop)
    {
        return NULL; // Erreur : allocation échouée
    }

    // Allocation des enfants pour condition et corps
    ast_loop->children = malloc(2 * sizeof(struct ast *));
    if (!ast_loop->children)
    {
        ast_free(ast_loop);
        return NULL; // Erreur : allocation échouée
    }
    ast_loop->children_count = 2;

    // Analyse de la condition
    ast_loop->children[0] = parse_command_list(lexer);
    if (!ast_loop->children[0])
    {
        ast_free(ast_loop);
        return NULL; // Erreur : échec du parsing de la condition
    }

    // Vérifie le token 'do'
    tok = lexer_peek(lexer);
    if (tok.type != TOKEN_DO)
    {
        ast_free(ast_loop);
        return NULL; // Erreur : 'do' attendu
    }
    lexer_pop(lexer);

    // Analyse du corps
    ast_loop->children[1] = parse_command_list(lexer);
    if (!ast_loop->children[1])
    {
        ast_free(ast_loop);
        return NULL; // Erreur : échec du parsing du corps
    }

    // Vérifie le token 'done'
    tok = lexer_peek(lexer);
    if (tok.type != TOKEN_DONE)
    {
        ast_free(ast_loop);
        return NULL; // Erreur : 'done' attendu
    }
    lexer_pop(lexer);

    return ast_loop; // Succès
}

struct ast *parser_parse(struct lexer *lexer)
{
    struct token tok = lexer_peek(lexer);

    if (tok.type == TOKEN_WHILE || tok.type == TOKEN_UNTIL)
    {
        return parse_while(lexer);
    }
    else if (tok.type == TOKEN_IF)
    {
        return parse_if_statement(lexer);
    }
    else
    {
        return parse_command_list(lexer);
    }
}

void ast_free(struct ast *node)
{
    if (!node)
        return;
    for (size_t i = 0; i < node->children_count; i++)
    {
        ast_free(node->children[i]);
    }
    free(node->children);

    if (node->type == AST_SIMPLE_COMMAND)
    {
        struct ast_command_data *data = node->data;
        if (data)
        {
            for (size_t i = 0; data->args && data->args[i]; i++)
            {
                free(data->args[i]);
            }
            free(data->args);
            free(data);
        }
    }
    else if (node->type == AST_IF)
    {
        struct ast_if_data *data = node->data;
        if (data)
        {
            ast_free(data->condition);
            ast_free(data->then_branch);
            ast_free(data->else_branch);
            free(data);
        }
    }
    free(node);
}

struct ast *parse_pipeline(struct lexer *lexer)
{
    struct ast *first_command = parse_simple_command(lexer);
    if (!first_command)
        return NULL;

    struct token tok = lexer_peek(lexer);
    if (tok.type != TOKEN_PIPE)
        return first_command;

    struct ast *pipeline_node = ast_create(AST_PIPELINE);
    if (!pipeline_node)
    {
        ast_free(first_command);
        return NULL;
    }

    pipeline_node->children = malloc(sizeof(struct ast *));
    if (!pipeline_node->children)
    {
        ast_free(pipeline_node);
        ast_free(first_command);
        return NULL;
    }

    pipeline_node->children[0] = first_command;
    pipeline_node->children_count = 1;

    while (tok.type == TOKEN_PIPE)
    {
        lexer_pop(lexer);
        struct ast *next_command = parse_simple_command(lexer);
        if (!next_command)
        {
            ast_free(pipeline_node);
            return NULL;
        }

        struct ast **new_children =
            realloc(pipeline_node->children,
                    sizeof(struct ast *) * (pipeline_node->children_count + 1));
        if (!new_children)
        {
            ast_free(next_command);
            ast_free(pipeline_node);
            return NULL;
        }

        pipeline_node->children = new_children;
        pipeline_node->children[pipeline_node->children_count++] = next_command;

        tok = lexer_peek(lexer);
    }

    return pipeline_node;
}

struct ast *parse_and_or(struct lexer *lexer)
{
    struct ast *left = parse_change(lexer); // Appel à parse_change
    if (!left)
        return NULL;

    struct token tok = lexer_peek(lexer);
    while (tok.type == TOKEN_AND || tok.type == TOKEN_OR)
    {
        lexer_pop(lexer);
        struct ast *right = parse_change(lexer); // Appel à parse_change ici aussi
        if (!right)
        {
            ast_free(left);
            return NULL;
        }

        struct ast *and_or_node = ast_create(AST_AND_OR);
        if (!and_or_node)
        {
            ast_free(left);
            ast_free(right);
            return NULL;
        }

        and_or_node->data = strdup(tok.type == TOKEN_AND ? "&&" : "||");
        and_or_node->children = malloc(2 * sizeof(struct ast *));
        if (!and_or_node->children)
        {
            ast_free(and_or_node);
            ast_free(left);
            ast_free(right);
            return NULL;
        }

        and_or_node->children[0] = left;
        and_or_node->children[1] = right;
        and_or_node->children_count = 2;

        left = and_or_node;
        tok = lexer_peek(lexer);
    }

    return left;
}