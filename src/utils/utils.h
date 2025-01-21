#ifndef UTILS_H
#define UTILS_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "args.h"
#include "args_count.h"
#include "oldpwd.h"
#include "pwd.h"
#include "variables.h"
#include "variables_count.h"

void set_verbose_mode(int enabled);
void verbose_log(const char *message);

// Gestion des variables
void set_variable(const char *name, const char *value);
const char *get_variable(const char *name);
char *substitute_variables(const char *input);
void free_variables(void);
void init_shell(void);

#endif /* !UTILS_H */
