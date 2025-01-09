#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "utils/utils.h"
#include "parser/ast.h"
#include "parser/parser.h"
#include "execution/exec.h"
#include "execution/builtins.h"
#include "lexer/lexer.h"

int main(int argc, char **argv) {
    int pretty_print = 0;
    char *command = NULL;
    FILE *input_file = NULL;

    // Si aucun argument n'est passé, quitter avec code 0
    if (argc == 1) {
        return 0;
    }

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
            print_arbre(ast, 0);
        } else {
            eval_ast(ast);
        }

        ast_free(ast);
    } else if (input_file) {
        char *line = NULL;
        size_t len = 0;
        if (getline(&line, &len, input_file) != -1) {
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
                fprintf(stderr, "Syntax error\n");
                free(line);
                fclose(input_file);
                return 2;
            }

            if (pretty_print) {
                print_arbre(ast, 0);
            } else {
                eval_ast(ast);
            }

            ast_free(ast);
        }
        free(line);
        fclose(input_file);
    } else {
        char *line = NULL;
        size_t len = 0;

        if (getline(&line, &len, stdin) != -1) {
            struct lexer *lexer = lexer_init(line);
            if (!lexer) {
                fprintf(stderr, "Failed to initialize lexer\n");
                free(line);
                return 2;
            }

            enum parser_status status;
            struct ast *ast = parser_parse(lexer, &status);
            lexer_destroy(lexer);

            if (status != PARSER_OK) {
                fprintf(stderr, "Syntax error\n");
                free(line);
                return 2;
            }

            if (pretty_print) {
                print_arbre(ast, 0);
            } else {
                eval_ast(ast);
            }

            ast_free(ast);
        }
        free(line);
    }

    return 0;
}
