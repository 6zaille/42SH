#include "utils.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>

#define _POSIX_C_SOURCE 200809L

int verbose_mode = 0;

void set_verbose_mode(int enabled) {
    verbose_mode = enabled;
}

void verbose_log(const char *message) {
    if (verbose_mode) {
        fprintf(stderr, "[VERBOSE]: %s\n", message);
    }
}

#define MAX_VARIABLES 256

struct variable {
    char *name;
    char *value;
};

static struct variable variables[MAX_VARIABLES];
static size_t variable_count = 0;

// Recherche une variable par son nom
static struct variable *find_variable(const char *name) {
    for (size_t i = 0; i < variable_count; i++) {
        if (strcmp(variables[i].name, name) == 0) {
            return &variables[i];
        }
    }
    return NULL;
}

// Ajoute ou met à jour une variable
void set_variable(const char *name, const char *value) {
    struct variable *var = find_variable(name);
    if (var) {
        free(var->value);
        var->value = xstrdup(value);
    } else {
        if (variable_count >= MAX_VARIABLES) {
            fprintf(stderr, "Error: maximum number of variables reached\n");
            return;
        }
        variables[variable_count].name = xstrdup(name);
        variables[variable_count].value = xstrdup(value);
        variable_count++;
    }
}

// Récupère la valeur d'une variable
const char *get_variable(const char *name) {
    struct variable *var = find_variable(name);
    return var ? var->value : NULL;
}

// Substitue les variables dans une chaîne de caractères
char *substitute_variables(const char *input) {
    char *result = malloc(4096); // Taille max d'une ligne
    if (!result) {
        perror("malloc");
        return NULL;
    }

    size_t result_pos = 0;
    for (const char *p = input; *p; p++) {
        if (*p == '$' && *(p + 1)) {
            const char *start = ++p;
            while (*p && (isalnum(*p) || *p == '_')) {
                p++;
            }
            size_t len = p - start;
            char *var_name = malloc(len + 1);
            if (!var_name) {
                perror("malloc");
                free(result);
                return NULL;
            }
            strncpy(var_name, start, len);
            var_name[len] = '\0';
            const char *value = get_variable(var_name);
            free(var_name);
            if (value) {
                size_t value_len = strlen(value);
                if (result_pos + value_len >= 4096) {
                    fprintf(stderr, "Error: substitution result too large\n");
                    free(result);
                    return NULL;
                }
                strcpy(&result[result_pos], value);
                result_pos += value_len;
            }
            p--;
        } else {
            if (result_pos >= 4095) {
                fprintf(stderr, "Error: substitution result too large\n");
                free(result);
                return NULL;
            }
            result[result_pos++] = *p;
        }
    }
    result[result_pos] = '\0';
    return result;
}

// Libère les variables
void free_variables(void) {
    for (size_t i = 0; i < variable_count; i++) {
        free(variables[i].name);
        free(variables[i].value);
    }
    variable_count = 0;
}
