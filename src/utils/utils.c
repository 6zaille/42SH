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