#define _POSIX_C_SOURCE 200809L

#include "parser.h"
#include "built_ins.h"
#include "executor.h"
#include "signals.h"
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <readline/readline.h>
#include <readline/history.h>

#define PROMPT "myShell> "

// Route commands for execution
static void process_commands(Command *cmds)
{
    if (!cmds) return;

    // Handle single built-in command
    if (is_builtin(cmds->argv[0]) && !cmds->next)
    {
        int saved_stdin  = dup(STDIN_FILENO);
        int saved_stdout = dup(STDOUT_FILENO);

        apply_redir(cmds);
        execute_builtin(cmds);

        dup2(saved_stdin, STDIN_FILENO);
        dup2(saved_stdout, STDOUT_FILENO);

        close(saved_stdin);
        close(saved_stdout);
    }
    // Handle single external command
    else if (!cmds->next)
    {
        execute_command(cmds);
    }
    // Handle pipeline (multiple commands)
    else
    {
        execute_pipeline(cmds);
    }
}

int main(void)
{
    char line[1024];

    setup_signals();

    while (1)
    {
        char *read_line = readline(PROMPT);

        // Handle EOF (Ctrl+D)
        if (!read_line)
        {
            printf("\n");
            break;
        }

        // Ignore empty input
        if (!*read_line)
        {
            free(read_line);
            continue;
        }

        // Add to readline's internal history for up/down arrows
        add_history(read_line);

        // Copy to our array buffer for parsing
        strncpy(line, read_line, sizeof(line) - 1);
        line[sizeof(line) - 1] = '\0';
        free(read_line);

        // Add to our custom history command
        add_to_history(line);

        // Parse and execute
        Command *cmds = parse_line(line);
        if (cmds)
        {
            process_commands(cmds);
            free_commands(cmds);
        }
    }

    return 0;
}
