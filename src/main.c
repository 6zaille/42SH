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
    FILE *input_file = NULL;

    printf("Parsing command-line arguments...\n");
    // Parse command-line arguments
    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "-c") == 0) {
            if (i + 1 >= argc) {
                fprintf(stderr, "Error: Missing argument for -c\n");
                fprintf(stderr, "Usage: 42sh [-c COMMAND] [FILE] [ARGUMENTS...]\n");
                return 2;
            }
            command = argv[++i];
        } else if (strcmp(argv[i], "--pretty-print") == 0) {
            pretty_print = 1;
        } else if (!input_file) {
            input_file = fopen(argv[i], "r");
            if (!input_file) {
                fprintf(stderr, "Error: Unable to open file %s\n", argv[i]);
                return 2;
            }
        } else {
            fprintf(stderr, "Error: Unexpected argument %s\n", argv[i]);
            fprintf(stderr, "Usage: 42sh [-c COMMAND] [FILE] [ARGUMENTS...]\n");
            return 2;
        }
    }

    if (command) {
        printf("Executing command: %s\n", command);
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
            printf("Pretty printing AST...\n");
            print_arbre(ast, 0);
        } else {
            printf("Evaluating AST...\n");
            eval_ast(ast);
        }

        ast_free(ast);
        return 0;
    } else if (input_file) {
        printf("Reading from input file...\n");
        char *line = NULL;
        size_t len = 0;

        while (getline(&line, &len, input_file) != -1) {
            printf("Processing line: %s\n", line);
            struct lexer *lexer = lexer_init(line);
            if (!lexer) {
                fprintf(stderr, "Failed to initialize lexer\n");
                free(line);
                fclose(input_file);
                return 2;
            }

            enum parser_status status;
            struct ast *ast = parser_parse(lexer, &status);
            lexer_destroy(lexer);

            if (status != PARSER_OK) {
                fprintf(stderr, "PARSER EST PAS OK\n");
                free(line);
                fclose(input_file);
                return 2;
            }

            if (pretty_print) {
                printf("Pretty printing AST...\n");
                print_arbre(ast, 0);
            } else {
                printf("Evaluating AST...\n");
                eval_ast(ast);
            }

            ast_free(ast);
        }

        free(line);
        fclose(input_file);
    } else {
        printf("Entering interactive mode...\n");
        interactive_mode();
    }

    return 0;
}

void interactive_mode() {
    char *line = NULL;
    size_t len = 0;

    printf("42sh> ");
    while (getline(&line, &len, stdin) != -1) {
        printf("Processing line: %s\n", line);
        struct lexer *lexer = lexer_init(line);
        if (!lexer) {
            fprintf(stderr, "Failed to initialize lexer\n");
            continue;
        }

        enum parser_status status;
        struct ast *ast = parser_parse(lexer, &status);
        lexer_destroy(lexer);

        if (status != PARSER_OK) {
            printf("ICI PROBLEME HUGO FONCTION PARSER TOUJOURS ERROR\n");
            fprintf(stderr, "Syntax error\n");
        } else {
            printf("Evaluating AST...\n");
            eval_ast(ast);
        }

        ast_free(ast);
        printf("42sh> ");
    }

    free(line);
}