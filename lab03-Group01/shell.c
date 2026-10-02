#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

#include "parser.h"

#define BUFLEN 1024
#define MAX_ARGS 128

extern char **environ;

int main(void)
{
    char buffer[BUFLEN];
    char parsedinput[BUFLEN];
    char *args[MAX_ARGS];

    printf("Welcome to the Group01 shell! Enter commands, enter 'quit' to exit\n");

    while (1) {
        printf("$ ");
        fflush(stdout);

        if (fgets(buffer, sizeof(buffer), stdin) == NULL) {
            if (feof(stdin)) {
                printf("\nBye!!\n");
                return 0;
            }

            perror("fgets");
            return 1;
        }

        trimstring(parsedinput, buffer, BUFLEN);

        // Ignore a blank command line.
        if (parsedinput[0] == '\0') {
            continue;
        }

        int argc = parseargs(parsedinput, args, MAX_ARGS);
        if (argc == -1) {
            fprintf(stderr, "Error: too many command-line arguments\n");
            continue;
        }
        if (argc == -2) {
            fprintf(stderr, "Error: unmatched quotation mark\n");
            continue;
        }
        if (argc == 0) {
            continue;
        }

        // "quit" is handled by the shell rather than passed to execve().
        if (strcmp(args[0], "quit") == 0) {
            printf("Bye!!\n");
            return 0;
        }

        pid_t child = fork();
        if (child < 0) {
            perror("fork");
            continue;
        }

        if (child == 0) {
            // args[0] MUST be the executable's full path.
            execve(args[0], args, environ);

            // execve() only returns if execution failed.
            perror("execve");
            _exit(127);
        }

        int status;
        if (waitpid(child, &status, 0) < 0) {
            perror("waitpid");
        }
    }
}
