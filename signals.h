// signals.h
// Module for handling Ctrl+C and Ctrl+Z in myShell

#ifndef SIGNALS_H
#define SIGNALS_H

#include <signal.h>
#include <sys/types.h>

// Keeps track of the foreground process PID
// Used by handlers to know which process to act on
extern volatile pid_t fg_child_pid;

// Gets set to 1 when SIGINT (Ctrl+C) is received
// Helps us break out of waitpid() cleanly
extern volatile sig_atomic_t sigint_received;

// Gets set to 1 when SIGTSTP (Ctrl+Z) is received
// Lets us know the process was suspended
extern volatile sig_atomic_t sigtstp_received;

// Call this once at the start of main() to set up signal handlers
void setup_signals(void);

// Called when user presses Ctrl+C
// Kills the foreground child and resets flags
void sigint_handler(int sig);

// Called when user presses Ctrl+Z
// Suspends the foreground child and resets flags
void sigtstp_handler(int sig);

#endif // SIGNALS_H
