#ifndef UTILS_H
#define UTILS_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void *xmalloc(size_t size);
char *xstrdup(const char *str);

void set_verbose_mode(int enabled);
void verbose_log(const char *message);

#endif /* !UTILS_H */
