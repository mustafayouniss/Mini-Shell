#ifndef EXECUTOR_H
#define EXECUTOR_H
/*
 * executor.h
 * Public interface for executor.c
*/

#include "parser.h"

void apply_redir(Command *cmd);
int execute_command(Command *cmd);

#endif /* EXECUTOR_H */
