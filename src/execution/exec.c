#include "exec.h"

#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

#include "../parser/ast.h"
#include "builtins.h"

#ifndef O_CLOEXEC
#    define O_CLOEXEC FD_CLOEXEC
#endif

struct saved_fd {
    int original_fd;
    int saved_fd;
};

static struct saved_fd *saved_fds = NULL;
static size_t saved_fd_count = 0;

static void save_fd(int fd) {
    struct saved_fd *new_saved_fds =
        realloc(saved_fds, (saved_fd_count + 1) * sizeof(struct saved_fd));
    if (!new_saved_fds) {
        perror("realloc");
        exit(EXIT_FAILURE);
    }
    saved_fds = new_saved_fds;
    saved_fds[saved_fd_count].original_fd = fd;
    saved_fds[saved_fd_count].saved_fd = dup(fd);
    if (saved_fds[saved_fd_count].saved_fd == -1) {
        perror("dup");
        exit(EXIT_FAILURE);
    }
    saved_fd_count++;
}

static void redirect_fd(int old_fd, int new_fd) {
    save_fd(new_fd);
    if (dup2(old_fd, new_fd) == -1) {
        perror("dup2");
        exit(EXIT_FAILURE);
    }
}

static void restore_fds() {
    for (size_t i = 0; i < saved_fd_count; i++) {
        if (dup2(saved_fds[i].saved_fd, saved_fds[i].original_fd) == -1) {
            perror("dup2");
        }
        close(saved_fds[i].saved_fd);
    }
    free(saved_fds);
    saved_fds = NULL;
    saved_fd_count = 0;
}

static void apply_redirection(const char *filename, int fd, int flags, int mode) {
    save_fd(fd);
    int new_fd = open(filename, flags, mode);
    if (new_fd == -1) {
        perror("open");
        exit(EXIT_FAILURE);
    }
    if (dup2(new_fd, fd) == -1) {
        perror("dup2");
        exit(EXIT_FAILURE);
    }
    close(new_fd);
}

int execute_command(int argc, char **argv) {
    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], ">") == 0) {
            if (i + 1 < argc) {
                apply_redirection(argv[i + 1], STDOUT_FILENO, O_WRONLY | O_CREAT | O_TRUNC, 0644);
                argv[i] = NULL;
                break;
            }
        } else if (strcmp(argv[i], ">>") == 0) {
            if (i + 1 < argc) {
                apply_redirection(argv[i + 1], STDOUT_FILENO, O_WRONLY | O_CREAT | O_APPEND, 0644);
                argv[i] = NULL;
                break;
            }
        } else if (strcmp(argv[i], "<") == 0) {
            if (i + 1 < argc) {
                apply_redirection(argv[i + 1], STDIN_FILENO, O_RDONLY, 0);
                argv[i] = NULL;
                break;
            }
        } else if (strcmp(argv[i], "2>") == 0) {
            if (i + 1 < argc) {
                apply_redirection(argv[i + 1], STDERR_FILENO, O_WRONLY | O_CREAT | O_TRUNC, 0644);
                argv[i] = NULL;
                break;
            }
        } else if (strcmp(argv[i], "<&") == 0) {
            if (i + 1 < argc) {
                int fd = atoi(argv[i + 1]);
                redirect_fd(fd, STDIN_FILENO);
                argv[i] = NULL;
                break;
            }
        } else if (strcmp(argv[i], ">&") == 0) {
            if (i + 1 < argc) {
                int fd = atoi(argv[i + 1]);
                redirect_fd(fd, STDOUT_FILENO);
                argv[i] = NULL;
                break;
            }
        } else if (strcmp(argv[i], "<>") == 0) {
            if (i + 1 < argc) {
                apply_redirection(argv[i + 1], STDIN_FILENO, O_RDWR | O_CREAT, 0644);
                argv[i] = NULL;
                break;
            }
        }
    }

    pid_t pid = fork();
    if (pid == 0) {
        // enfant
        if (execvp(argv[0], argv) == -1) {
            perror("42sh");
            exit(EXIT_FAILURE);
        }
    } else if (pid > 0) {
        // parent
        int status;
        waitpid(pid, &status, 0);
        restore_fds();

        return WEXITSTATUS(status);
    } else {
        perror("fork");
        return 1;
    }

    return 0;
}

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