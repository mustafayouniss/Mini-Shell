#ifndef EXECUTOR_H
#define EXECUTOR_H
#include "signals.h"
/*
 * executor.h
 * Public interface for executor.c
*/

#include "parser.h"
int execute_pipeline(Command *head);
int execute_command(Command *cmd);
void apply_redir(Command *cmd);

#endif /* EXECUTOR_H */
