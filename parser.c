#define _POSIX_C_SOURCE 200809L
#include "parser.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#define MAX_ARGS 64
#define MAX_TOKEN 1024

static Command* create_command(void) {
    Command *cmd = calloc(1, sizeof(Command));
    cmd->argv = calloc(MAX_ARGS, sizeof(char*));
    return cmd;
}

static char* safe_strdup(const char *s) {
    if (!s) return NULL;
    char *dup = malloc(strlen(s) + 1);
    if (dup) strcpy(dup, s);
    return dup;
}

static int is_operator(char c) {
    return c == '|' || c == '>' || c == '<' || c == '&';
}

static void add_argument(Command *cmd, const char *token) {
    if (cmd->argc < MAX_ARGS - 1) {
        cmd->argv[cmd->argc++] = safe_strdup(token);
    }
}

Command* parse_line(const char *input) {
    if (!input || !*input) return NULL;

    Command *head = create_command();
    Command *current = head;
    int len = strlen(input);
    char token[MAX_TOKEN];
    
    int tok_len = 0;
    int is_inside_quote = 0;
    char quote_char = 0;
    int expecting_file = 0;
    int pending_input = 0;

    for (int i = 0; i <= len; i++) {
        char current_char = input[i];

        // Handle quoting
        if (current_char == '"' || current_char == '\'') {
            if (!is_inside_quote) {
                is_inside_quote = 1;
                quote_char = current_char;
                continue;
            } else if (current_char == quote_char) {
                is_inside_quote = 0;
                quote_char = 0;
                continue;
            }
        }

        // Handle token separation outside quotes
        if (!is_inside_quote && (isspace(current_char) || is_operator(current_char) || current_char == '\0')) {
            if (tok_len > 0) {
                token[tok_len] = '\0';
                
                if (expecting_file) {
                    if (pending_input) {
                        current->input_redir = safe_strdup(token);
                    } else {
                        current->output_redir = safe_strdup(token);
                    }
                    expecting_file = 0;
                } else {
                    add_argument(current, token);
                }
                tok_len = 0;
            }

            // Handle operators
            if (current_char == '|') {
                current->next = create_command();
                current = current->next;
            } else if (current_char == '<') {
                pending_input = 1;
                expecting_file = 1;
            } else if (current_char == '>') {
                if (input[i + 1] == '>') {
                    current->append = true;
                    i++;
                }
                pending_input = 0;
                expecting_file = 1;
            } else if (current_char == '&') {
                current->background = true;
            }
            continue;
        }

        // Build token
        if (tok_len < MAX_TOKEN - 1) {
            token[tok_len++] = current_char;
        }
    }

    // Ensure all commands are NULL-terminated for execvp
    for (Command *cmd = head; cmd; cmd = cmd->next) {
        cmd->argv[cmd->argc] = NULL;
    }
    
    return head;
}

void free_commands(Command *head) {
    while (head) {
        Command *next = head->next;
        for (int i = 0; i < head->argc; i++) {
            free(head->argv[i]);
        }
        free(head->argv);
        free(head->input_redir);
        free(head->output_redir);
        free(head);
        head = next;
    }
}