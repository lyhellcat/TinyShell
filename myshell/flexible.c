#define _GNU_SOURCE

#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <termio.h>
#include <unistd.h>
#include <ctype.h>

int main(int argc, char *argv[]) {
    char *pathvar[] = {"PATH=/bin:/usr/bin", "TERM=xterm", NULL};
    int is_dig = 1;
    for (int i = 0; i < strlen(argv[1]); i++) {
        if (!isdigit(argv[1][i])) {
            is_dig = 0;
        }
    }
    if (is_dig) {
        execvpe("sleep", argv, pathvar);
    } else {
        execvpe("echo", argv, pathvar);
    }
}
