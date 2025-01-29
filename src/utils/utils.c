#define _POSIX_C_SOURCE 200809L

#include "utils.h"

#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>

#include "../parser/ast.h"
#include "args.h"

// GLOBALS
char *args[256];

static struct variable *find_variable(const char *name)
{
    for (size_t i = 0; i < variable_count; i++)
    {
        if (strcmp(variables[i].name, name) == 0)
        {
            return &variables[i];
        }
    }
    return NULL;
}

void init_shell(void)
{
    char buffer[4096];
    if (getcwd(buffer, sizeof(buffer)))
    {
        oldpwd = pwd;
        pwd = strdup(buffer);
        set_variable("PWD", pwd);
        set_variable("OLDPWD", "");
    }
}

void set_variable(const char *name, const char *value)
{
    struct variable *var = find_variable(name);
    if (var)
    {
        free(var->value);
        var->value = strdup(value);
    }
    else
    {
        if (variable_count >= MAX_VARIABLES)
        {
            fprintf(stderr, "Error: maximum number of variables reached\n");
            return;
        }
        variables[variable_count].name = strdup(name);
        variables[variable_count].value = strdup(value);
        variable_count++;
    }
}

const char *get_variable(const char *name)
{
    static char buffer[4096];
    if (strcmp(name, "?") == 0)
    {
        snprintf(buffer, sizeof(buffer), "%d", last_exit_status);
        return buffer;
    }
    else if (strcmp(name, "$") == 0)
    {
        snprintf(buffer, sizeof(buffer), "%d", getpid());
        return buffer;
    }
    else if (strcmp(name, "#") == 0)
    {
        snprintf(buffer, sizeof(buffer), "%d", args_count);
        return buffer;
    }
    else if (strcmp(name, "@") == 0 || strcmp(name, "*") == 0)
    {
        if (args_count == 0)
        {
            return NULL;
        }
        size_t pos = 0;
        for (int i = 0; i < args_count; i++)
        {
            pos += snprintf(buffer + pos, sizeof(buffer) - pos, "%s%c", args[i],
                            '\0');
        }
        return buffer;
    }
    else if (strcmp(name, "RANDOM") == 0)
    {
        srand(time(NULL));
        snprintf(buffer, sizeof(buffer), "%d", rand() % 32768);
        return buffer;
    }
    else if (strcmp(name, "UID") == 0)
    {
        snprintf(buffer, sizeof(buffer), "%d", getuid());
        return buffer;
    }
    else if (strcmp(name, "PWD") == 0)
    {
        return pwd;
    }
    else if (strcmp(name, "OLDPWD") == 0)
    {
        return oldpwd;
    }
    struct variable *var = find_variable(name);
    return var ? var->value : NULL;
}

/*char *substitute_variables(const char *input)
{
    char *result = malloc(4096);
    if (!result)
    {
        perror("malloc");
        return NULL;
    }

    size_t result_pos = 0;
    for (const char *p = input; *p; p++)
    {
        if (*p == '$' && *(p + 1))
        {
            const char *start = ++p;
            while (*p && (isalnum(*p) || *p == '_'))
            {
                p++;
            }
            size_t len = p - start;
            char *var_name = malloc(len + 1);
            if (!var_name)
            {
                perror("malloc");
                free(result);
                return NULL;
            }
            strncpy(var_name, start, len);
            var_name[len] = '\0';
            const char *value = get_variable(var_name);
            free(var_name);
            if (value)
            {
                size_t value_len = strlen(value);
                if (result_pos + value_len >= 4096)
                {
                    fprintf(stderr, "Error: substitution result too large\n");
                    free(result);
                    return NULL;
                }
                strcpy(&result[result_pos], value);
                result_pos += value_len;
            }
            p--;
        }
        else
        {
            if (result_pos >= 4095)
            {
                fprintf(stderr, "Error: substitution result too large\n");
                free(result);
                return NULL;
            }
            result[result_pos++] = *p;
        }
    }
    result[result_pos] = '\0';
    return result;
}*/

/*void set_args(size_t count, char **arguments)
{
    args_count = count;
    for (size_t i = 0; i < count; i++)
    {
        args[i] = strdup(arguments[i]);
    }
}*/

/*void free_variables(void)
{
    for (size_t i = 0; i < variable_count; i++)
    {
        free(variables[i].name);
        free(variables[i].value);
    }
    for (size_t i = 0; i < args_count; i++)
    {
        free(args[i]);
    }
    variable_count = 0;
    args_count = 0;
    free(pwd);
    free(oldpwd);
}
*/
