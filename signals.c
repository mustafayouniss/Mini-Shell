#define _POSIX_C_SOURCE 200809L

#include "signals.h"
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/wait.h>
#include <errno.h>

volatile pid_t fg_child_pid = 0;
volatile sig_atomic_t sigint_received = 0;
volatile sig_atomic_t sigtstp_received = 0;

// Handle Ctrl+C (kill foreground)
void sigint_handler(int sig)
{
    (void)sig;
    sigint_received = 1;
    
    const char newline[] = "\n";
    write(STDOUT_FILENO, newline, sizeof(newline) - 1);
    
    if (fg_child_pid > 0) {
        kill(fg_child_pid, SIGINT);
    }
}

// Handle Ctrl+Z (suspend foreground)
void sigtstp_handler(int sig)
{
    (void)sig;
    sigtstp_received = 1;
    
    const char newline[] = "\n";
    write(STDOUT_FILENO, newline, sizeof(newline) - 1);
    
    if (fg_child_pid > 0) {
        kill(fg_child_pid, SIGTSTP);
    }
}

// Clean up finished background processes
static void sigchld_handler(int sig)
{
    (void)sig;
    
    int saved_errno = errno; 
    
    while (waitpid(-1, NULL, WNOHANG) > 0) {
        // Clear finished processes
    }
    
    errno = saved_errno;
}

// Initialize signal handlers
void setup_signals(void)
{
    struct sigaction sa_int, sa_tstp, sa_chld;
    
    // SIGINT
    sa_int.sa_handler = sigint_handler;
    sa_int.sa_flags = 0;
    sigemptyset(&sa_int.sa_mask);
    if (sigaction(SIGINT, &sa_int, NULL) == -1) {
        perror("Failed to set SIGINT handler");
        exit(EXIT_FAILURE);
    }
    
    // SIGTSTP
    sa_tstp.sa_handler = sigtstp_handler;
    sa_tstp.sa_flags = 0;
    sigemptyset(&sa_tstp.sa_mask);
    if (sigaction(SIGTSTP, &sa_tstp, NULL) == -1) {
        perror("Failed to set SIGTSTP handler");
        exit(EXIT_FAILURE);
    }

    // SIGCHLD
    sa_chld.sa_handler = sigchld_handler;
    sa_chld.sa_flags = SA_RESTART | SA_NOCLDSTOP; 
    sigemptyset(&sa_chld.sa_mask);
    if (sigaction(SIGCHLD, &sa_chld, NULL) == -1) {
        perror("Failed to set SIGCHLD handler");
        exit(EXIT_FAILURE);
    }
}
