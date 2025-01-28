#include "exec.h"

#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

#include "../parser/ast.h"
#include "../utils/pwd.h"
#include "builtins.h"

#ifndef O_CLOEXEC
#    define O_CLOEXEC FD_CLOEXEC
#endif /* O_CLOEXEC */

struct saved_fd
{
    int original_fd;
    int saved_fd;
};
extern int loop_running;
static struct saved_fd *saved_fds = NULL;
static size_t saved_fd_count = 0;

// GLOBALS
char *pwd = NULL;

static void save_fd(int fd);
static void restore_fds(void);
static void apply_redirection(const char *filename, int fd, int flags,
                              int mode);
static void redirect_fd(int old_fd, int new_fd);

static void save_fd(int fd)
{
    struct saved_fd *new_saved_fds =
        realloc(saved_fds, (saved_fd_count + 1) * sizeof(struct saved_fd));
    /*if (!new_saved_fds)
    {
        perror("realloc");
        exit(EXIT_FAILURE);
    }*/
    saved_fds = new_saved_fds;
    saved_fds[saved_fd_count].original_fd = fd;
    saved_fds[saved_fd_count].saved_fd = dup(fd);
    if (saved_fds[saved_fd_count].saved_fd == -1)
    {
        perror("dup");
        exit(EXIT_FAILURE);
    }
    saved_fd_count++;
}

static void restore_fds(void)
{
    for (size_t i = 0; i < saved_fd_count; i++)
    {
        if (dup2(saved_fds[i].saved_fd, saved_fds[i].original_fd) == -1)
        {
            perror("dup2");
        }
        close(saved_fds[i].saved_fd);
    }
    free(saved_fds);
    saved_fds = NULL;
    saved_fd_count = 0;
}

static void apply_redirection(const char *filename, int fd, int flags, int mode)
{
    save_fd(fd);
    int new_fd = open(filename, flags, mode);
    if (new_fd == -1)
    {
        //perror("open");
        exit(EXIT_FAILURE);
    }
    if (dup2(new_fd, fd) == -1)
    {
        //perror("dup2");
        //close(new_fd);
        exit(EXIT_FAILURE);
    }
    close(new_fd);
}

static void redirect_fd(int old_fd, int new_fd)
{
    save_fd(new_fd);
    if (dup2(old_fd, new_fd) == -1)
    {
        //perror("dup2");
        exit(EXIT_FAILURE);
    }
}

int handle_redirection(char **argv, int i, redirection_t redir, int append_mode)
{
    if (strcmp(argv[i], redir.symbol) == 0 && argv[i + 1])
    {
        int flags = redir.flags;
        if (append_mode)
        {
            flags |= O_APPEND;
        }
        apply_redirection(argv[i + 1], redir.fd, flags, redir.mode);
        argv[i] = NULL;
        argv[i + 1] = NULL;
        return 1;
    }
    return 0;
}

int handle_fd_redirection(char **argv, int i, const char *symbol, int target_fd)
{
    if (strcmp(argv[i], symbol) == 0 && argv[i + 1])
    {
        int fd = atoi(argv[i + 1]);
        redirect_fd(fd, target_fd);
        argv[i] = NULL;
        argv[i + 1] = NULL;
        return 1;
    }
    return 0;
}

int handle_pid(int pid, char **argv)
{
    if (pid == 0)
    {
        if (execvp(argv[0], argv) == -1)
        {
            perror("42sh");
            exit(127);
        }
    }
    else if (pid > 0)
    {
        int status;
        waitpid(pid, &status, 0);
        restore_fds();
        if (WIFEXITED(status))
        {
            return WEXITSTATUS(status);
        }
    }
    else
    {
        perror("fork");
        return 1;
    }
    return 0;
}

int execute_command(int argc, char **argv)
{
    for (int i = 0; i < argc; i++)
    {
        if (argv[i] == NULL)
            continue;

        if (handle_redirection(argv, i,
                               (redirection_t){ ">", STDOUT_FILENO,
                                                O_WRONLY | O_CREAT | O_TRUNC,
                                                0644 },
                               0)
            || handle_redirection(argv, i,
                                  (redirection_t){ ">>", STDOUT_FILENO,
                                                   O_WRONLY | O_CREAT, 0644 },
                                  1)
            || handle_redirection(
                argv, i, (redirection_t){ "<", STDIN_FILENO, O_RDONLY, 0 }, 0)
            || handle_redirection(argv, i,
                                  (redirection_t){ "2>", STDERR_FILENO,
                                                   O_WRONLY | O_CREAT | O_TRUNC,
                                                   0644 },
                                  0)
            || handle_redirection(
                argv, i,
                (redirection_t){ "<>", STDIN_FILENO, O_RDWR | O_CREAT, 0644 },
                0)
            || handle_fd_redirection(argv, i, "<&", STDIN_FILENO)
            || handle_fd_redirection(argv, i, ">&", STDOUT_FILENO))
        {
            break;
        }
    }

    int j = 0;
    for (int i = 0; i < argc; i++)
    {
        if (argv[i] != NULL)
        {
            argv[j++] = argv[i];
        }
    }
    argv[j] = NULL;

    int builtin_status = execute_builtin(j, argv);
    if (builtin_status != -1)
    {
        restore_fds();
        return builtin_status;
    }

    pid_t pid = fork();

    return handle_pid(pid, argv);
}

int execute_builtin(int argc, char **argv)
{
    if (argc == 0 || argv == NULL)
    {
        return 1;
    }

    if (strcmp(argv[0], "echo") == 0)
    {
        return builtin_echo(argc, argv);
    }
    if (strcmp(argv[0], "true") == 0)
    {
        return builtin_true();
    }
    if (strcmp(argv[0], "false") == 0)
    {
        return builtin_false();
    }
    if (strcmp(argv[0], "exit") == 0)
    {
        return builtin_exit(argc, argv);
    }
    /*if (strcmp(argv[0], "break") == 0)
    {
        loop_running = 0; // Interrompt la boucle
        return 0;
    }*/
    return -1;
}
