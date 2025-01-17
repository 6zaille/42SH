#ifndef UTILS_H
#define UTILS_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAX_VARIABLES 256

struct variable
{
    char *name;
    char *value;
};

extern struct variable variables[MAX_VARIABLES];
extern size_t variable_count;


void set_verbose_mode(int enabled);
void verbose_log(const char *message);

// Gestion des variables
void set_variable(const char *name, const char *value);
const char *get_variable(const char *name);
char *substitute_variables(const char *input);
void free_variables(void);

#endif /* !UTILS_H */
