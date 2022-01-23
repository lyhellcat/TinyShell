#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

int main(void) {
    int number1, number2, sum;

    int input_fds = open("./input.txt", O_RDONLY);

    int stdin_fd = dup(STDIN_FILENO);

    if (dup2(input_fds, STDIN_FILENO) < 0) {
        printf("Unable to duplicate file descriptor.");
        exit(EXIT_FAILURE);
    }

    scanf("%d %d", &number1, &number2);

    sum = number1 + number2;

    printf("%d + %d = %d\n", number1, number2, sum);

    dup2(stdin_fd, STDIN_FILENO);

    int result;
    scanf("%d", &result);
    printf("%d\n", result);

    return EXIT_SUCCESS;
}
