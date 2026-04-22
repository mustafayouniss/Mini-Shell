#define _POSIX_C_SOURCE 200809L

#include "executor.h"
#include "signals.h"
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

static void handle_background(pid_t pid)
{
    printf("[bg] %d\n", pid);
}

static int wait_foreground(pid_t pid)
{
    int status = 0;

    fg_child_pid = pid;

    if (waitpid(pid, &status, 0) < 0)
    {
        if (errno != EINTR)
            perror("waitpid");
    }

    fg_child_pid = 0;

    if (WIFEXITED(status))
        return WEXITSTATUS(status);

    return 1;
}
/* ------------------------------------------------------------------ */
/* Background & Foreground Process Management                */
/*                                                                    */
/* 1) Background Execution:                                            */
/*    - If command ends with '&', the shell must NOT wait for it.      */
/*    - We simply print the PID and return immediately to the prompt. */
/*                                                                    */
/* 2) Foreground Execution:                                            */
/*    - If command runs normally, the shell must wait until it ends.  */
/*    - We store its PID in fg_child_pid so SIGINT handler (Ctrl+C)   */
/*      can terminate ONLY the foreground process without exiting the */
/*      shell itself.                                                 */
/*                                                                    */
/* This design keeps the code clean by separating waiting logic into  */
/* a helper function (wait_foreground) and background printing logic  */
/* into another helper (handle_background).                           */
/* ------------------------------------------------------------------ */
int execute_pipeline(Command *head)
{
    int num_cmds = 0;
    for (Command *c = head; c; c = c->next)
        num_cmds++;

    if (num_cmds == 0)
        return 0;

    int pipes[num_cmds - 1][2];
    pid_t pids[num_cmds];

    /* Create pipes */
    for (int i = 0; i < num_cmds - 1; i++)
    {
        if (pipe(pipes[i]) == -1)
        {
            perror("pipe");
            return 1;
        }
    }

    /* Fork each command */
    for (int i = 0; i < num_cmds; i++)
    {
        Command *cmd = head;
        for (int j = 0; j < i; j++)
            cmd = cmd->next;

        pids[i] = fork();

        if (pids[i] < 0)
        {
            perror("fork");
            exit(1);
        }

        if (pids[i] == 0)
        {
            /* Child process */
            signal(SIGINT, SIG_DFL);
            signal(SIGTSTP, SIG_DFL);

            /* connect input */
            if (i > 0)
                dup2(pipes[i - 1][0], STDIN_FILENO);

            /* connect output */
            if (i < num_cmds - 1)
                dup2(pipes[i][1], STDOUT_FILENO);

            /* close all pipes */
            for (int j = 0; j < num_cmds - 1; j++)
            {
                close(pipes[j][0]);
                close(pipes[j][1]);
            }

            /* apply redirection */
            apply_redir(cmd);

            execvp(cmd->argv[0], cmd->argv);
            perror(cmd->argv[0]);
            exit(127);
        }
    }

    /* Parent closes all pipes */
    for (int i = 0; i < num_cmds - 1; i++)
    {
        close(pipes[i][0]);
        close(pipes[i][1]);
    }

    /* Determine background flag (last command) */
    Command *last = head;
    while (last->next)
        last = last->next;

    /* Background pipeline: do not wait */
    if (last->background)
    {
        handle_background(pids[num_cmds - 1]);
        return 0;
    }

    /* Foreground pipeline: wait all */
    int status = 0;
    fg_child_pid = pids[num_cmds - 1];

    for (int i = 0; i < num_cmds; i++)
        waitpid(pids[i], &status, 0);

    fg_child_pid = 0;

    return 0;
}


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
        signal(SIGINT, SIG_DFL);
        signal(SIGTSTP, SIG_DFL);

        apply_redir(cmd);
        execvp(cmd->argv[0], cmd->argv);

        if (errno == ENOENT)
            fprintf(stderr, "myShell: %s: command not found\n", cmd->argv[0]);
        else
            perror(cmd->argv[0]);

        exit(127);
    }

    /* ── Parent ── */
    if (cmd->background) {
        handle_background(pid);
        return 0;
    }

    return wait_foreground(pid);
}
