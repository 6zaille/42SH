#define _POSIX_C_SOURCE 200809L

#include "parser.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "../lexer/lexer.h"
#include "../lexer/token.h"
#include "ast.h"

static struct ast *ast_create(enum ast_type type)
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

static struct ast *ast_create_if(struct ast *condition, struct ast *then_branch,
                                 struct ast *else_branch)
{
    struct ast *node = ast_create(AST_IF);
    if (!node)
    {
        return NULL;
    }
    struct ast_if_data *data = malloc(sizeof(*data));
    if (!data)
    {
        free(node);
        return NULL;
    }
    data->condition = condition;
    data->then_branch = then_branch;
    data->else_branch = else_branch;
    node->data = data;

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
    return type == TOKEN_IF || type == TOKEN_THEN || type == TOKEN_ELSE ||
           type == TOKEN_ELIF || type == TOKEN_FI;
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
    lexer_pop(lexer); // Passe au token suivant
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
    while (tok.type == TOKEN_WORD || tok.type == TOKEN_VARIABLE || tok.type == TOKEN_ASSIGNMENT
           || (tok.type >= TOKEN_REDIRECT_IN && tok.type <= TOKEN_REDIRECT_RW) || is_special_token(tok.type))
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
    struct token tok = lexer_peek(lexer);

    // Cas de négation
    if (tok.type == TOKEN_NEGATION)
    {
        lexer_pop(lexer);
        struct ast *child =
            parse_change(lexer); // Récursivité pour analyser ce qui suit
        if (!child)
            return NULL;

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

    // Analyse d'un pipeline ou d'une commande simple
    return parse_pipeline(lexer);
}

static struct ast *parse_command_list(struct lexer *lexer)
{
    struct ast *list_node = ast_create(AST_LIST);
    if (!list_node)
        return NULL;

    struct ast **commands = NULL;
    size_t count = 0;

    while (1)
    {
        struct ast *command_node = parse_change(lexer);
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

        struct token tok = lexer_peek(lexer);
        if (tok.type == TOKEN_SEMICOLON)
        {
            lexer_pop(lexer);
        }
        else if (tok.type == TOKEN_EOF || tok.type == TOKEN_NEWLINE)
        {
            break;
        }
        
        else
        {
            
            ast_free(list_node);
            return NULL;
        }
    }

    list_node->children = commands;
    list_node->children_count = count;
    return list_node;
}

static struct ast *parse_if_condition(struct lexer *lexer)
{
    struct ast *condition = ast_create(AST_LIST);
    if (!condition)
        return NULL;

    condition->children = NULL;
    condition->children_count = 0;

    while (1)
    {
        struct token tok = lexer_peek(lexer);
        if (tok.type == TOKEN_THEN)
        {
            lexer_pop(lexer);
            break;
        }

        if (tok.type == TOKEN_EOF || tok.type == TOKEN_FI)
        {
            fprintf(stderr, "mauvais token fin dans if condition\n");
            ast_free(condition);
            return NULL;
        }

        struct ast *cmd = parse_command_list(lexer);
        if (!cmd)
        {
            fprintf(stderr, "mauvaise commande dans if condition\n");
            ast_free(condition);
            return NULL;
        }

        condition->children =
            realloc(condition->children,
                    sizeof(struct ast *) * (condition->children_count + 1));
        if (!condition->children)
        {
            perror("realloc");
            ast_free(condition);
            return NULL;
        }
        condition->children[condition->children_count++] = cmd;

        tok = lexer_peek(lexer);
        if (tok.type == TOKEN_SEMICOLON || tok.type == TOKEN_NEWLINE)
        {
            lexer_pop(lexer);
        }
        else if (tok.type != TOKEN_THEN)
        {
            fprintf(stderr, "token anormal dans if'%s'\n",
                    tok.value ? tok.value : "NULL");
            ast_free(condition);
            return NULL;
        }
    }

    return condition;
}

static struct ast *parse_then(struct lexer *lexer)
{
    struct ast *condition = ast_create(AST_LIST);
    if (!condition)
        return NULL;

    condition->children = NULL;
    condition->children_count = 0;

    while (1)
    {
        struct token tok = lexer_peek(lexer);
        if (tok.type == TOKEN_ELSE || tok.type == TOKEN_FI || tok.type == TOKEN_ELIF)
        {
            //lexer_pop(lexer);
            break;
        }

        if (tok.type == TOKEN_EOF)
        {
            fprintf(stderr, "probleme de fin dans parse_then\n");
            ast_free(condition);
            return NULL;
        }

        struct ast *cmd = parse_command_list(lexer);
        if (!cmd)
        {
            fprintf(stderr, "commande incorrect\n");
            ast_free(condition);
            return NULL;
        }

        condition->children =
            realloc(condition->children,
                    sizeof(struct ast *) * (condition->children_count + 1));
        if (!condition->children)
        {
            perror("realloc");
            ast_free(condition);
            return NULL;
        }
        condition->children[condition->children_count++] = cmd;

        tok = lexer_peek(lexer);
        if (tok.type == TOKEN_SEMICOLON || tok.type == TOKEN_NEWLINE)
        {
            lexer_pop(lexer);
        }
        else if (tok.type != TOKEN_THEN)
        {
            fprintf(stderr, "token anormal dans then'%s'\n",
                    tok.value ? tok.value : "NULL");
            ast_free(condition);
            return NULL;
        }
    }

    return condition;
}

static struct ast *parse_else(struct lexer *lexer)
{
    struct ast *condition = ast_create(AST_LIST);
    if (!condition)
        return NULL;

    condition->children = NULL;
    condition->children_count = 0;

    while (1)
    {
        struct token tok = lexer_peek(lexer);
        if (tok.type == TOKEN_FI)
        {
            //lexer_pop(lexer);
            break;
        }

        if (tok.type == TOKEN_EOF)
        {
            fprintf(stderr, "probleme de fin dans else\n");
            ast_free(condition);
            return NULL;
        }

        struct ast *cmd = parse_command_list(lexer);
        if (!cmd)
        {
            fprintf(stderr, "commande invalide dans parse_else\n");
            ast_free(condition);
            return NULL;
        }

        condition->children =
            realloc(condition->children,
                    sizeof(struct ast *) * (condition->children_count + 1));
        if (!condition->children)
        {
            perror("realloc");
            ast_free(condition);
            return NULL;
        }
        condition->children[condition->children_count++] = cmd;

        tok = lexer_peek(lexer);
        if (tok.type == TOKEN_SEMICOLON || tok.type == TOKEN_NEWLINE)
        {
            lexer_pop(lexer);
        }
        else if (tok.type != TOKEN_THEN)
        {
            fprintf(stderr, "token anormal dans else'%s'\n",
                    tok.value ? tok.value : "NULL");
            ast_free(condition);
            return NULL;
        }
    }

    return condition;
}

static struct ast *parse_if_statement(struct lexer *lexer)
{
    struct token tok = lexer_peek(lexer);
    if (tok.type != TOKEN_IF)
    {
        fprintf(stderr, "Syntax error: expected 'if', got '%s'\n", tok.value ? tok.value : "NULL");
        return NULL;
    }
    lexer_pop(lexer); // Consomme 'if'

    // Analyse la condition du bloc if
    struct ast *condition = parse_if_condition(lexer);
    if (!condition)
        return NULL;

    // Analyse le bloc then
    struct ast *then_branch = parse_then(lexer);
    if (!then_branch)
    {
        ast_free(condition);
        return NULL;
    }

    // Gestion des blocs elif et else
    struct ast *else_branch = NULL;

    while (1)
    {
        tok = lexer_peek(lexer);

        // Cas d'un bloc `elif`
        if (tok.type == TOKEN_ELIF)
        {
            lexer_pop(lexer); // Consomme 'elif'

            // Analyse la condition du bloc elif
            struct ast *elif_condition = parse_if_condition(lexer);
            if (!elif_condition)
            {
                ast_free(condition);
                ast_free(then_branch);
                return NULL;
            }

            // Analyse le bloc then de l'elif
            struct ast *elif_then = parse_then(lexer);
            if (!elif_then)
            {
                ast_free(condition);
                ast_free(then_branch);
                ast_free(elif_condition);
                return NULL;
            }

            // Crée un nœud AST pour l'elif et l'ajoute dans la branche else
            struct ast *elif_node = ast_create_if(elif_condition, elif_then, NULL);
            if (!elif_node)
            {
                ast_free(condition);
                ast_free(then_branch);
                ast_free(elif_condition);
                ast_free(elif_then);
                return NULL;
            }

            if (!else_branch)
            {
                else_branch = elif_node; // Premier elif devient le else_branch
            }
            else
            {
                // Ajoute cet elif comme branche else du précédent elif
                struct ast_if_data *current_else_data = (struct ast_if_data *)else_branch->data;
                while (current_else_data->else_branch && current_else_data->else_branch->type == AST_IF)
                {
                    current_else_data = (struct ast_if_data *)current_else_data->else_branch->data;
                }
                current_else_data->else_branch = elif_node;
            }
        }
        else
            break; // Pas de token elif, on passe à la suite
    }

    // Cas d'un bloc `else` (optionnel)
    tok = lexer_peek(lexer);
    if (tok.type == TOKEN_ELSE)
    {
        lexer_pop(lexer); // Consomme 'else'

        struct ast *else_body = parse_else(lexer);
        if (!else_body)
        {
            ast_free(condition);
            ast_free(then_branch);
            ast_free(else_branch);
            return NULL;
        }

        if (!else_branch)
        {
            else_branch = else_body; // Si pas de elif, le else devient else_branch
        }
        else
        {
            // Ajoute le else au dernier elif
            struct ast_if_data *current_else_data = (struct ast_if_data *)else_branch->data;
            while (current_else_data->else_branch && current_else_data->else_branch->type == AST_IF)
            {
                current_else_data = (struct ast_if_data *)current_else_data->else_branch->data;
            }
            current_else_data->else_branch = else_body;
        }
    }

    // Vérifie et consomme le token 'fi'
    tok = lexer_peek(lexer);
    if (tok.type != TOKEN_FI)
    {
        fprintf(stderr, "Syntax error: expected 'fi', got '%s'\n", tok.value ? tok.value : "NULL");
        ast_free(condition);
        ast_free(then_branch);
        ast_free(else_branch);
        return NULL;
    }
    lexer_pop(lexer); // Consomme 'fi'

    return ast_create_if(condition, then_branch, else_branch);
}


struct ast *parser_parse(struct lexer *lexer)
{
    struct token tok = lexer_peek(lexer);

    if (tok.type == TOKEN_IF)
    {
        return parse_if_statement(lexer);
    }
    return parse_command_list(lexer);
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