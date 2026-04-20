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
    return s ? strdup(s) : NULL;
}

static int is_operator(char c) {
    return c == '|' || c == '>' || c == '<' || c == '&';
}

Command* parse_line(const char *input) {
    if (!input || !*input) return NULL;

    Command *head = create_command();
    Command *current = head;
    int i = 0, len = strlen(input);
    char token[MAX_TOKEN];
    int tok_len = 0, in_quote = 0;
    char quote_char = 0, expecting_file = 0, pending_input = 0;

    while (i <= len) {
        char c = input[i];
        if (c == '"' || c == '\'') {
            if (!in_quote) { in_quote = 1; quote_char = c; i++; continue; }
            else if (c == quote_char) { in_quote = 0; quote_char = 0; i++; continue; }
        }
        if (!in_quote && (isspace(c) || is_operator(c) || c == '\0')) {
            if (tok_len > 0) {
                token[tok_len] = '\0';
                if (expecting_file) {
                    if (pending_input) current->input_redir = safe_strdup(token);
                    else current->output_redir = safe_strdup(token);
                    expecting_file = 0;
                } else {
                    if (current->argc < MAX_ARGS - 1)
                        current->argv[current->argc++] = safe_strdup(token);
                }
                tok_len = 0;
            }
            if (c == '|') {
                current->next = create_command();
                current = current->next;
            } else if (c == '<') {
                pending_input = 1; expecting_file = 1;
            } else if (c == '>') {
                if (input[i + 1] == '>') { current->append = true; i++; }
                pending_input = 0; expecting_file = 1;
            } else if (c == '&') {
                current->background = true;
            }
            i++; continue;
        }
        if (tok_len < MAX_TOKEN - 1) token[tok_len++] = c;
        i++;
    }
    for (Command *cmd = head; cmd; cmd = cmd->next)
        cmd->argv[cmd->argc] = NULL;
    return head;
}

void free_commands(Command *head) {
    while (head) {
        Command *next = head->next;
        for (int i = 0; i < head->argc; i++) free(head->argv[i]);
        free(head->argv);
        free(head->input_redir);
        free(head->output_redir);
        free(head);
        head = next;
    }
}