#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "exec.h"
#include "builtins.h"

int execute_builtin(int argc, char **argv) {
    if (argc == 0 || argv == NULL) {
        return 1;
    }

    if (strcmp(argv[0], "echo") == 0) {
        return builtin_echo(argc, argv);
    }
    if (strcmp(argv[0], "true") == 0) {
        return builtin_true();
    }
    if (strcmp(argv[0], "false") == 0) {
        return builtin_false();
    }
    if (strcmp(argv[0], "exit") == 0) {
        return builtin_exit(argc, argv);
    }

    return -1; // Indique que ce n'est pas un builtin.
}

int execute_command(int argc, char **argv) {
    int builtin_status = execute_builtin(argc, argv);
    if (builtin_status != -1) {
        return builtin_status;
    }

    pid_t pid = fork();
    if (pid == 0) {
        if (execvp(argv[0], argv) == -1) {
            perror("42sh");
            exit(127);
        }
    } else if (pid > 0) {
        int status;
        waitpid(pid, &status, 0);
        if (WIFEXITED(status)) {
            return WEXITSTATUS(status);
        }
    } else {
        perror("fork");
        return 1;
    }
    return 0;
}