#define _POSIX_C_SOURCE 200809L

#include "executor.h"

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <signal.h>
#include <errno.h>

/* ------------------------------------------------------------------ */
/* apply_redir()                                                        */
/* Called inside the child process, BEFORE execvp().                  */
/* Opens redirection files and wires them over stdin/stdout.          */
/* ------------------------------------------------------------------ */
void apply_redir(Command *cmd)
{
    /* ── Input redirection: command < file ── */
    if (cmd->input_redir) {
        int fd = open(cmd->input_redir, O_RDONLY);
        if (fd < 0) {
            perror(cmd->input_redir);   /* e.g. "input.txt: No such file or directory" */
            exit(1);
        }
        if (dup2(fd, STDIN_FILENO) < 0) {
            perror("dup2 input");
            exit(1);
        }
        close(fd);   /* fd no longer needed; stdin now points to the file */
    }

    /* ── Output redirection: command > file  or  command >> file ── */
    if (cmd->output_redir) {
        int flags = O_WRONLY | O_CREAT;
        flags |= cmd->append ? O_APPEND : O_TRUNC;

        int fd = open(cmd->output_redir, flags, 0644);
        if (fd < 0) {
            perror(cmd->output_redir);
            exit(1);
        }
        if (dup2(fd, STDOUT_FILENO) < 0) {
            perror("dup2 output");
            exit(1);
        }
        close(fd);   /* fd no longer needed; stdout now points to the file */
    }
}

/* ------------------------------------------------------------------ */
/* execute_command()                                                    */
/* fork + apply_redir + execvp + waitpid for one external command.    */
/* ------------------------------------------------------------------ */
int execute_command(Command *cmd)
{
    if (!cmd || !cmd->argv || !cmd->argv[0]) return 0;

    pid_t pid = fork();
    if (pid < 0) {
        perror("fork");
        return 1;
    }

    if (pid == 0) {
        /* ── Child ── */
        signal(SIGINT, SIG_DFL);    /* restore default so Ctrl+C kills child */
        apply_redir(cmd);           /* hook up < and > before exec */
        execvp(cmd->argv[0], cmd->argv);

        /* execvp only returns on error */
        if (errno == ENOENT)
            fprintf(stderr, "myShell: %s: command not found\n", cmd->argv[0]);
        else
            perror(cmd->argv[0]);
        exit(127);
    }

    /* ── Parent ── */
    if (cmd->background) {
        printf("[bg] %d\n", pid);
        return 0;
    }

    int status = 0;
    if (waitpid(pid, &status, 0) < 0 && errno != EINTR)
        perror("waitpid");

    return WIFEXITED(status) ? WEXITSTATUS(status) : 1;
}
