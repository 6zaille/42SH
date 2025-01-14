#include "exec.h"

#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

#include "builtins.h"

#ifndef O_CLOEXEC
#    define O_CLOEXEC FD_CLOEXEC
#endif

struct saved_fd
{
    int original_fd;
    int saved_fd;
};

static struct saved_fd *saved_fds = NULL;
static size_t saved_fd_count = 0;

static void save_fd(int fd)
{
    struct saved_fd *new_saved_fds =
        realloc(saved_fds, (saved_fd_count + 1) * sizeof(struct saved_fd));
    if (!new_saved_fds)
    {
        perror("realloc");
        exit(EXIT_FAILURE);
    }
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

static void restore_fds()
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
        perror("open");
        exit(EXIT_FAILURE);
    }
    if (dup2(new_fd, fd) == -1)
    {
        perror("dup2");
        exit(EXIT_FAILURE);
    }
    close(new_fd);
}

static void redirect_fd(int source_fd, int target_fd)
{
    save_fd(target_fd);
    if (dup2(source_fd, target_fd) == -1)
    {
        perror("dup2");
        exit(EXIT_FAILURE);
    }
}

void execute_with_redirections(struct ast *node)
{
    if (!node || node->type != AST_SIMPLE_COMMAND)
    {
        return;
    }

    struct ast_command_data *data = (struct ast_command_data *)node->data;
    if (!data || !data->redirections)
    {
        return;
    }

    for (size_t i = 0; i < data->redirection_count; i++)
    {
        struct redirection *redir = &data->redirections[i];
        switch (redir->type)
        {
        case REDIR_OUT:
            apply_redirection(redir->filename, STDOUT_FILENO,
                              O_WRONLY | O_CREAT | O_TRUNC, 0644);
            break;
        case REDIR_IN:
            apply_redirection(redir->filename, STDIN_FILENO, O_RDONLY, 0);
            break;
        case REDIR_APPEND:
            apply_redirection(redir->filename, STDOUT_FILENO,
                              O_WRONLY | O_CREAT | O_APPEND, 0644);
            break;
        case REDIR_DUP_OUT:
            redirect_fd(atoi(redir->filename), STDOUT_FILENO);
            break;
        case REDIR_DUP_IN:
            redirect_fd(atoi(redir->filename), STDIN_FILENO);
            break;
        case REDIR_CLOBBER:
            apply_redirection(redir->filename, STDOUT_FILENO,
                              O_WRONLY | O_CREAT | O_TRUNC | O_CLOEXEC, 0644);
            break;
        case REDIR_RW:
            apply_redirection(redir->filename, STDIN_FILENO, O_RDWR | O_CREAT,
                              0644);
            break;
        default:
            fprintf(stderr, "Unsupported redirection type\n");
            exit(EXIT_FAILURE);
        }
    }
}

int execute_command_with_redirections(int argc, char **argv, struct ast *node)
{
    execute_with_redirections(node);

    int status = execute_command(argc, argv);

    restore_fds();

    return status;
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
    return -1; // Indique que ce n'est pas un builtin.
}

int execute_command(int argc, char **argv)
{
    int builtin_status = execute_builtin(argc, argv);
    if (builtin_status != -1)
    {
        return builtin_status; // Retourne immédiatement pour les builtins
    }

    pid_t pid = fork();
    if (pid == 0)
    {
        if (execvp(argv[0], argv) == -1)
        {
            perror("42sh");
            exit(127); // Retourne 127 en cas d'échec d'exécution
        }
    }
    else if (pid > 0)
    {
        int status;
        waitpid(pid, &status, 0);
        if (WIFEXITED(status))
        {
            return WEXITSTATUS(
                status); // Retourne le statut de sortie du processus
        }
        else
        {
            return 1; // Retourne 1 en cas de problème
        }
    }
    else
    {
        perror("fork");
        return 1; // Retourne 1 si le fork échoue
    }
    return 0; // Défaut (ne devrait pas être atteint)
}
