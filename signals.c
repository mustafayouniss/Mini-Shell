// signals.c
// Signal handling for Ctrl+C (SIGINT) and Ctrl+Z (SIGTSTP)

#define _POSIX_C_SOURCE 200809L
#include "signals.h"
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/wait.h>
#include <errno.h>

// Global variables
volatile pid_t fg_child_pid = 0;
volatile sig_atomic_t sigint_received = 0;
volatile sig_atomic_t sigtstp_received = 0;

// Handler for Ctrl+C - kills foreground child
void sigint_handler(int sig) {
    (void)sig;  // Suppress unused parameter warning
    sigint_received = 1;
    
    // Print newline safely (write is async-signal-safe)
    const char newline[] = "\n";
    write(STDOUT_FILENO, newline, sizeof(newline) - 1);
    
    // Kill the foreground child if it exists
    if (fg_child_pid > 0) {
        kill(fg_child_pid, SIGINT);
    }
}

// Handler for Ctrl+Z - suspends foreground child
void sigtstp_handler(int sig) {
    (void)sig;  // Suppress unused parameter warning
    sigtstp_received = 1;
    
    // Print newline safely
    const char newline[] = "\n";
    write(STDOUT_FILENO, newline, sizeof(newline) - 1);
    
    // Suspend the foreground child if it exists
    if (fg_child_pid > 0) {
        kill(fg_child_pid, SIGTSTP);
    }
}

// Handler for SIGCHLD - reaps background child processes (prevents zombies)
static void sigchld_handler(int sig) {
    (void)sig; // Suppress unused parameter warning
    
    // Save errno because waitpid might change it, and we don't want to mess up the main program
    int saved_errno = errno; 
    
    // WNOHANG means "clean up any dead children, but DON'T wait if they are still running"
    // -1 means "any child process"
    while (waitpid(-1, NULL, WNOHANG) > 0) {
        // Just looping to clear all finished background processes
    }
    
    errno = saved_errno; // Restore errno
}

// Setup signal handlers using sigaction
void setup_signals(void) {
    struct sigaction sa_int, sa_tstp;
    
    // Setup Ctrl+C handler
    sa_int.sa_handler = sigint_handler;
    sa_int.sa_flags = 0;  // No SA_RESTART - we want waitpid() interrupted
    sigemptyset(&sa_int.sa_mask);
    if (sigaction(SIGINT, &sa_int, NULL) == -1) {
        perror("Failed to set SIGINT handler");
        exit(EXIT_FAILURE);
    }
    
    // Setup Ctrl+Z handler
    sa_tstp.sa_handler = sigtstp_handler;
    sa_tstp.sa_flags = 0;
    sigemptyset(&sa_tstp.sa_mask);
    if (sigaction(SIGTSTP, &sa_tstp, NULL) == -1) {
        perror("Failed to set SIGTSTP handler");
        exit(EXIT_FAILURE);
    }

    // Setup SIGCHLD handler
    struct sigaction sa_chld;
    sa_chld.sa_handler = sigchld_handler;
    // SA_RESTART: if a system call is interrupted, restart it. 
    // SA_NOCLDSTOP: only notify us when child terminates, not when it stops/pauses.
    sa_chld.sa_flags = SA_RESTART | SA_NOCLDSTOP; 
    sigemptyset(&sa_chld.sa_mask);
    
    if (sigaction(SIGCHLD, &sa_chld, NULL) == -1) {
        perror("Failed to set SIGCHLD handler");
        exit(EXIT_FAILURE);
    }
}
