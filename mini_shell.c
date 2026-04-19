#define _POSIX_C_SOURCE 200809L
#include "parser.h"
#include "builtins.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>          // <-- ADDED: for open(), O_RDONLY, etc.
#include <sys/types.h>
#include <sys/wait.h>
#include <signal.h>
#include <errno.h>

#define PROMPT "myShell> "


// Requirement 6: Ctrl+C terminates foreground child only
static void sigint_handler(int sig) {
    (void)sig;  // <-- ADDED: suppress unused parameter warning
}


static void setup_redirection(Command *cmd) {
    if (cmd->input_redir) {
        int fd = open(cmd->input_redir, O_RDONLY);
        if (fd < 0) { perror(cmd->input_redir); exit(1); }
        dup2(fd, STDIN_FILENO);
        close(fd);
    }
    if (cmd->output_redir) {
        int flags = O_WRONLY | O_CREAT | O_TRUNC;
        if (cmd->append) flags |= O_APPEND;
        int fd = open(cmd->output_redir, flags, 0644);
        if (fd < 0) { perror(cmd->output_redir); exit(1); }
        dup2(fd, STDOUT_FILENO);
        close(fd);
    }
}

static void execute_pipeline(Command *head) {
    int num_cmds = 0;
    for (Command *c = head; c; c = c->next) num_cmds++;
    if (num_cmds == 0) return;

    int pipes[num_cmds - 1][2];
    pid_t pids[num_cmds];

    for (int i = 0; i < num_cmds - 1; i++)
        if (pipe(pipes[i]) == -1) { perror("pipe"); return; }

    for (int i = 0; i < num_cmds; i++) {
        Command *cmd = head;
        for (int j = 0; j < i; j++) cmd = cmd->next;

        pids[i] = fork();
        if (pids[i] == -1) { perror("fork"); exit(1); }

        if (pids[i] == 0) {
            signal(SIGINT, SIG_DFL);
            if (i > 0) { dup2(pipes[i - 1][0], STDIN_FILENO); close(pipes[i - 1][0]); }
            if (i < num_cmds - 1) { dup2(pipes[i][1], STDOUT_FILENO); close(pipes[i][1]); }
            for (int j = 0; j < num_cmds - 1; j++) { close(pipes[j][0]); close(pipes[j][1]); }
            setup_redirection(cmd);
            execvp(cmd->argv[0], cmd->argv);
            perror(cmd->argv[0]);
            exit(127);
        }
    }

    for (int i = 0; i < num_cmds - 1; i++) {
        close(pipes[i][0]);
        close(pipes[i][1]);
    }

    Command *last = head;
    for (int i = 0; i < num_cmds - 1; i++) last = last->next;

    if (last->background) {
        // <-- FIXED: use pids[] array, not non-existent last->pid
        printf("[%d] %d\n", getpid(), pids[num_cmds - 1]);
    } else {
        signal(SIGINT, SIG_IGN);
        for (int i = 0; i < num_cmds; i++)
            waitpid(pids[i], NULL, 0);
        signal(SIGINT, sigint_handler);
    }
}

int main(void) {
    char line[1024];
    signal(SIGINT, sigint_handler);

    while (1) {
        printf("%s", PROMPT);
        if (!fgets(line, sizeof(line), stdin)) break;
        line[strcspn(line, "\n")] = '\0';
        if (!*line) continue;

        add_to_history(line);
        Command *cmds = parse_line(line);
        if (!cmds) continue;

        if (is_builtin(cmds->argv[0]) && !cmds->next) {
            int saved_stdin = dup(STDIN_FILENO);
            int saved_stdout = dup(STDOUT_FILENO);

            setup_redirection(cmds);
            execute_builtin(cmds);

            dup2(saved_stdin, STDIN_FILENO);
            dup2(saved_stdout, STDOUT_FILENO);

            close(saved_stdin);
            close(saved_stdout);
        } else {
            execute_pipeline(cmds);
        }
        free_commands(cmds);
    }
    return 0;
}
