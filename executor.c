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
#include <time.h>

// Setup I/O redirection
void apply_redir(Command *cmd)
{
    // Input redirection
    if (cmd->input_redir) {
        int fd = open(cmd->input_redir, O_RDONLY);
        if (fd < 0) {
            perror(cmd->input_redir);
            exit(1);
        }
        if (dup2(fd, STDIN_FILENO) < 0) {
            perror("dup2 input");
            exit(1);
        }
        close(fd);
    }

    // Output redirection
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
        close(fd);
    }
}

static void handle_background(pid_t pid)
{
    printf("[bg] %d\n", pid);
    
    // Add a tiny delay so very fast background commands (like 'ls')
    // can print their output before our next prompt is drawn.
    struct timespec ts = {0, 50000000}; // 50 milliseconds
    nanosleep(&ts, NULL);
}

static int wait_foreground(pid_t pid)
{
    int status = 0;

    fg_child_pid = pid;

    if (waitpid(pid, &status, 0) < 0) {
        if (errno != EINTR) {
            perror("waitpid");
        }
    }

    fg_child_pid = 0;

    if (WIFEXITED(status)) {
        return WEXITSTATUS(status);
    }

    return 1;
}

// Run one pipeline command
static void execute_pipeline_child(Command *cmd, int i, int num_cmds, int pipes[][2])
{
    // Reset signals to default behavior in the child
    signal(SIGINT, SIG_DFL);
    signal(SIGTSTP, SIG_DFL);

    // Connect stdin from the previous pipe
    if (i > 0) {
        dup2(pipes[i - 1][0], STDIN_FILENO);
    }

    // Connect stdout to the next pipe
    if (i < num_cmds - 1) {
        dup2(pipes[i][1], STDOUT_FILENO);
    }

    // Close all pipes in this child
    for (int j = 0; j < num_cmds - 1; j++) {
        close(pipes[j][0]);
        close(pipes[j][1]);
    }

    apply_redir(cmd);
    execvp(cmd->argv[0], cmd->argv);
    
    perror(cmd->argv[0]);
    exit(127);
}

int execute_pipeline(Command *head)
{
    int num_cmds = 0;
    for (Command *c = head; c; c = c->next) {
        num_cmds++;
    }

    if (num_cmds == 0) return 0;

    int pipes[num_cmds - 1][2];
    pid_t pids[num_cmds];

    // Create all pipes
    for (int i = 0; i < num_cmds - 1; i++) {
        if (pipe(pipes[i]) == -1) {
            perror("pipe");
            return 1;
        }
    }

    // Fork and execute each command in the pipeline
    Command *cmd = head;
    for (int i = 0; i < num_cmds; i++) {
        pids[i] = fork();

        if (pids[i] < 0) {
            perror("fork");
            exit(1);
        }

        if (pids[i] == 0) {
            execute_pipeline_child(cmd, i, num_cmds, pipes);
        }
        
        cmd = cmd->next;
    }

    // Parent must close all pipe ends
    for (int i = 0; i < num_cmds - 1; i++) {
        close(pipes[i][0]);
        close(pipes[i][1]);
    }

    // Check if the pipeline should run in the background
    Command *last = head;
    while (last->next) {
        last = last->next;
    }

    if (last->background) {
        handle_background(pids[num_cmds - 1]);
        return 0;
    }

    // Wait for all foreground processes in the pipeline
    int status = 0;
    fg_child_pid = pids[num_cmds - 1];

    for (int i = 0; i < num_cmds; i++) {
        waitpid(pids[i], &status, 0);
    }

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
        // Child execution
        signal(SIGINT, SIG_DFL);
        signal(SIGTSTP, SIG_DFL);

        apply_redir(cmd);
        execvp(cmd->argv[0], cmd->argv);

        if (errno == ENOENT) {
            fprintf(stderr, "myShell: %s: command not found\n", cmd->argv[0]);
        } else {
            perror(cmd->argv[0]);
        }
        exit(127);
    }

    // Parent execution
    if (cmd->background) {
        handle_background(pid);
        return 0;
    }

    return wait_foreground(pid);
}
