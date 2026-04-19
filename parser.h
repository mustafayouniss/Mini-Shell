#ifndef PARSER_H
#define PARSER_H

#include <stdbool.h>

typedef struct Command {
    char **argv;
    int argc;
    char *input_redir;
    char *output_redir;
    bool append;
    bool background;
    struct Command *next;
} Command;

Command* parse_line(const char *input);
void free_commands(Command *head);
bool is_builtin(const char *cmd);

#endif