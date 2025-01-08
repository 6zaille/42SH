#include <stdio.h>
#include <stdlib.h>
#include "lexer.h"
#include "parser.h"
#include "ast.h"

void print_token(struct token *tok)
{
    if (!tok)
        return;

    switch (tok->type)
    {
    case TOKEN_IF:
        printf("TOKEN_IF");
        break;
    case TOKEN_THEN:
        printf("TOKEN_THEN");
        break;
    case TOKEN_ELIF:
        printf("TOKEN_ELIF");
        break;
    case TOKEN_ELSE:
        printf("TOKEN_ELSE");
        break;
    case TOKEN_FI:
        printf("TOKEN_FI");
        break;
    case TOKEN_SEMICOLON:
        printf("TOKEN_SEMICOLON");
        break;
    case TOKEN_NEWLINE:
        printf("TOKEN_NEWLINE");
        break;
    case TOKEN_WORD:
        printf("TOKEN_WORD");
        break;
    case TOKEN_SINGLE_QUOTE:
        printf("TOKEN_SINGLE_QUOTE");
        break;
    case TOKEN_EOF:
        printf("TOKEN_EOF");
        break;
    case TOKEN_ERROR:
        printf("TOKEN_ERROR");
        break;
    default:
        printf("UNKNOWN");
    }

    if (tok->value)
        printf(", Value: '%s'", tok->value);
    printf("\n");
}

void print_ast(struct ast *node, int depth)
{
    if (!node)
        return;

    for (int i = 0; i < depth; ++i)
        printf("  ");

    switch (node->type)
    {
    case AST_NUMBER:
        printf("AST_NUMBER");
        break;
    default:
        printf("UNKNOWN");
    }

    if (node->value)
        printf(", Value: %ld", node->value);

    printf("\n");

    print_ast(node->left, depth + 1);
    print_ast(node->right, depth + 1);
}

void test_parser(const char *input)
{
    struct lexer *lexer = lexer_init(input);
    if (!lexer)
        return;

    enum parser_status status = PARSER_OK;
    struct ast *root = parser_parse(lexer, &status);

    if (status == PARSER_OK)
        print_ast(root, 0);

    ast_free(root);
    lexer_destroy(lexer);
}

int main(void)
{
    test_parser("if foo then bar fi");
    test_parser("if foo then bar else baz fi");
    test_parser("if foo then bar elif baz then qux fi");
    test_parser("foo;");
    test_parser("");
    test_parser("if foo then");

    return EXIT_SUCCESS;
}
