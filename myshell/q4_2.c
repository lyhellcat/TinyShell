#include <ctype.h>
#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <termio.h>
#include <unistd.h>

int main() {
    int cnt = 0;
    for (;;) {
        cnt++;
        if (cnt > 10000) {  // gets stuck
            while (1) {     // Running

            }
        }
    }
}
