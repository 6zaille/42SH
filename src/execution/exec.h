#ifndef EXEC_H
#define EXEC_H

#include "../parser/ast.h"
#include "../parser/parser.h"

typedef struct
{
    const char *symbol;
    int fd;
    int flags;
    int mode;
} redirection_t;

struct saved_fd
{
    int original_fd;
    int saved_fd;
};

extern int loop_running;

void apply_redirection(const char *filename, int fd, int flags, int mode);
int execute_builtin(int argc, char **argv);
int execute_command(int argc, char **argv);
int execute_command_with_redirections(int argc, char **argv, struct ast *node);
void execute_with_redirections(struct ast *node);

#endif /* !EXEC_H */
