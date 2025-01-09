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

    printf("Debug: Starting main function\n");

    // Handle options (this part is missing in your code, you need to implement it)
    // For example, you can parse command line arguments here
    // option_status = handle_options(argc, argv, &pretty_print, &command);

    int option_status = 0; // Assuming handle_options sets this to 0 on success
    if (option_status != 0) {
        printf("Debug: handle_options returned %d\n", option_status);
        return option_status;
    }

    if (command) {
        printf("Debug: Command mode with command: %s\n", command);
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
            printf("Debug: Pretty printing AST\n");
            print_arbre(ast, 0);
        } else {
            printf("Debug: Evaluating AST\n");
            eval_ast(ast);
        }

        ast_free(ast);
        return 0;
    }

    printf("Debug: Entering interactive mode\n");
    interactive_mode();
    return 0;
}

void interactive_mode() {
    char *line = NULL;
    size_t len = 0;

    printf("42sh> ");
    while (getline(&line, &len, stdin) != -1) {
        printf("Debug: Read line: %s\n", line);
        struct lexer *lexer = lexer_init(line);
        if (!lexer) {
            fprintf(stderr, "Failed to initialize lexer\n");
            continue;
        }
        printf("Debug: Lexer initialized\n");
        printf("Debug lexr %s \n",lexer->input);
        enum parser_status status;
        struct ast *ast = parser_parse(lexer, &status);
        printf("%d\n",status);
        printf("Debug: Parser parsed\n");
        lexer_destroy(lexer);
        printf("Debug: Lexer destroyed\n");
        if (status != PARSER_OK) {
            fprintf(stderr, "Syntax error\n");
        } else {
            printf("Debug: Evaluating AST\n");
            eval_ast(ast);
        }

        ast_free(ast);
        printf("42sh> ");
    }

    free(line);
}
