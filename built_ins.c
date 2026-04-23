#define _POSIX_C_SOURCE 200809L
#include "built_ins.h"
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#define MAX_HISTORY 50

static char *history[MAX_HISTORY];
static int history_count = 0;

// Add command to history
void add_to_history(const char *cmd)
{
    if (!cmd || !*cmd) return;

    if (history_count == MAX_HISTORY) {
        free(history[0]);
        memmove(history, history + 1, (MAX_HISTORY - 1) * sizeof(char*));
        history_count--;
    }

    char *dup = malloc(strlen(cmd) + 1);
    if (dup) {
        strcpy(dup, cmd);
    }
    history[history_count++] = dup;
}

// Print command history
void print_history(void)
{
    for (int i = 0; i < history_count; i++) {
        printf("%4d  %s\n", i + 1, history[i]);
    }
}

// Check if built-in command
bool is_builtin(const char *cmd)
{
    if (!cmd) return false;
    
    return strcmp(cmd, "cd") == 0 ||
           strcmp(cmd, "exit") == 0 ||
           strcmp(cmd, "pwd") == 0 ||
           strcmp(cmd, "history") == 0;
}

// Execute built-in command
int execute_builtin(Command *cmd)
{
    if (!cmd || !cmd->argv[0]) return 1;

    if (strcmp(cmd->argv[0], "exit") == 0) {
        exit(0);
    }

    if (strcmp(cmd->argv[0], "pwd") == 0) {
        char buf[1024];
        if (getcwd(buf, sizeof(buf))) {
            printf("%s\n", buf);
        } else {
            perror("pwd");
        }
        return 1;
    }

    if (strcmp(cmd->argv[0], "cd") == 0) {
        const char *dir = cmd->argc > 1 ? cmd->argv[1] : getenv("HOME");

        if (!dir) {
            fprintf(stderr, "cd: HOME not set\n");
            return 1;
        }

        if (chdir(dir) != 0) {
            perror("cd");
        }
        return 1;
    }

    if (strcmp(cmd->argv[0], "history") == 0) {
        print_history();
        return 1;
    }

    return 0;
}
