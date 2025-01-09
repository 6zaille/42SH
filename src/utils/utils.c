#include <stdio.h>
#include <string.h>
#include "utils.h"

int verbose_mode = 0;

void set_verbose_mode(int enabled) {
    verbose_mode = enabled;
}

void verbose_log(const char *message) {
    if (verbose_mode) {
        fprintf(stderr, "[VERBOSE]: %s\n", message);
    }
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
        } else if (strcmp(argv[i], "--pretty-print") == 0) {
            *pretty_print = 1;
        } else {
            fprintf(stderr, "42sh: unrecognized option '%s'\n", argv[i]);
            return 2;
        }
    }
    return 0;
}