#ifndef BUILTINS_H
#define BUILTINS_H

#include "parser.h"

// check if command is builtin
bool is_builtin(const char *cmd);

// execute builtin command
int execute_builtin(Command *cmd);

// history
void add_to_history(const char *cmd);
void print_history(void);

#endif
