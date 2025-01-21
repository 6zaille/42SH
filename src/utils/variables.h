#ifndef VARIABLES_H
#define VARIABLES_H

struct variable {
    char *name;
    char *value;
};

#define MAX_VARIABLES 256
extern struct variable variables[MAX_VARIABLES];

#endif /* !VARIABLES_H */