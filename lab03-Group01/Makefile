CC = gcc
CFLAGS = -Wall -Wextra

shell.out: shell.c parser.c parser.h
	$(CC) $(CFLAGS) shell.c parser.c -o shell.out

clean:
	rm -f shell.out