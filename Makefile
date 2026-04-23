CC = gcc
CFLAGS = -Wall -Wextra -g -std=c99 -D_POSIX_C_SOURCE=200809L

SRCS = myShell.c built_ins.c executor.c parser.c signals.c
OBJS = $(SRCS:.c=.o)
EXEC = myShell

all: $(EXEC)

$(EXEC): $(OBJS)
	$(CC) $(CFLAGS) -o $@ $^ -lreadline

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f $(OBJS) $(EXEC)
