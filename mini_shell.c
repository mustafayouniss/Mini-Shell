#define _POSIX_C_SOURCE 200809L

#include "parser.h"
#include "builtins.h"
#include "executor.h"
#include "signals.h"
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#define PROMPT "myShell> "

int main(void)
{
    char line[1024];

    setup_signals();

    while (1)
    {
        printf("%s", PROMPT);
        fflush(stdout);

        if (!fgets(line, sizeof(line), stdin))
        {
            // If the read was interrupted by a signal (e.g., Ctrl+C)
            if (errno == EINTR) {
                clearerr(stdin); // Clear the error indicator to allow reading the next line
                printf("\n");    // Print a newline for a clean prompt display
                continue;        // Continue the loop, do not exit the shell
            }
            break; // Exit the loop on EOF (e.g., when the user presses Ctrl+D)
        }

        line[strcspn(line, "\n")] = '\0';
        if (!*line)
            continue;

        add_to_history(line);

        Command *cmds = parse_line(line);
        if (!cmds)
            continue;

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
        else if (!cmds->next)
        {
            execute_command(cmds);
        }
        else
        {
            execute_pipeline(cmds);
        }

        free_commands(cmds);
    }

    return 0;
}