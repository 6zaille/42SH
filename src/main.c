#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "utils/utils.h"
#include "parser/ast.h"
#include "parser/parser.h"
#include "execution/exec.h"

int handle_c_option(char *command) {
    if (!command || strlen(command) == 0) {
        fprintf(stderr, "42sh: option -c requires a non-empty command\n");
        return 2;
    }

    verbose_log("Parsing command...");
    struct ast *ast = parse_input_from_string(command);
    if (!ast) {
        fprintf(stderr, "42sh: syntax error in command: %s\n", command);
        return 2;
    }

    verbose_log("Executing command...");
    int status = execute_ast(ast);
    ast_free(ast);
    return status;
}

int handle_options(int argc, char **argv, char **command, int *pretty_print) {
    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "-c") == 0) {
            if (i + 1 < argc) {
                *command = argv[++i];
            } else {
                fprintf(stderr, "42sh: missing argument after -c\n");
                return 2;
            }
        } else if (strcmp(argv[i], "--verbose") == 0) {
            set_verbose_mode(1);
        } else if (strcmp(argv[i], "--pretty-print") == 0) {
            *pretty_print = 1;
        } else {
            fprintf(stderr, "42sh: unrecognized option '%s'\n", argv[i]);
            return 2;
        }
    }
    return 0;
}

int main(int argc, char **argv) {
    int pretty_print = 0;
    char *command = NULL;

    int option_status = handle_options(argc, argv, &command, &pretty_print);
    if (option_status != 0) {
        return option_status;
    }

    if (command) {
        return handle_c_option(command);
    }

    printf("Starting interactive mode...\n");
    return 0;
}
