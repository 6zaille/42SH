#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "utils/utils.h"
#include "parser/ast.h"
#include "parser/parser.h"
#include "execution/exec.h"
#include "execution/builtins.h"
#include "lexer/lexer.h"

void interactive_mode();

int main(int argc, char **argv) {
    int pretty_print = 0;
    char *command = NULL;

    int option_status = handle_options(argc, argv, &command, &pretty_print);
    if (option_status != 0) {
        return option_status;
    }

    if (command) {
        struct lexer *lexer = lexer_init(command);
        if (!lexer) {
            fprintf(stderr, "Failed to initialize lexer\n");
            return 2;
        }

        enum parser_status status;
        struct ast *ast = parser_parse(lexer, &status);
        lexer_destroy(lexer);

        if (status != PARSER_OK) {
            fprintf(stderr, "Syntax error\n");
            return 2;
        }

        if (pretty_print) {
            print_arbre(ast);
        } else {
            eval_ast(ast);
        }

        ast_free(ast);
        return 0;
    }

    interactive_mode();
    return 0;
}

void interactive_mode() {
    char *line = NULL;
    size_t len = 0;

    printf("42sh> ");
    while (getline(&line, &len, stdin) != -1) {
        struct lexer *lexer = lexer_init(line);
        if (!lexer) {
            fprintf(stderr, "Failed to initialize lexer\n");
            continue;
        }

        enum parser_status status;
        struct ast *ast = parser_parse(lexer, &status);
        lexer_destroy(lexer);

        if (status != PARSER_OK) {
            fprintf(stderr, "Syntax error\n");
        } else {
            eval_ast(ast);
        }

        ast_free(ast);
        printf("42sh> ");
    }

    free(line);
}
