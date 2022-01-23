#define _GNU_SOURCE
#include <fcntl.h>
#include <stddef.h>
#include <sys/wait.h>
#include <unistd.h>
const int READ_END = 0;
const int WRITE_END = 1;

static pid_t run_process(char *exe, char *arg1, int std_in, int std_out) {
    pid_t child = fork();
    if (child == 0) {  // fork child process
        char *argv[] = {exe, arg1, NULL};
        if (std_in != -1)  // redirect stdin
            dup2(std_in, STDIN_FILENO);
        if (std_out != -1)  // redirect stdout
            dup2(std_out, STDOUT_FILENO);
        execvp(exe, argv);
    }
    waitpid(child, NULL, 0);  // wait for child
    return child;
}

int main(int ac, char *av[]) {
    int fd[2];
    pipe2(fd, O_CLOEXEC);
    run_process("cat", av[1], -1, fd[WRITE_END]);
    close(fd[WRITE_END]);  // close parent's write end

    run_process("wc", "-m", fd[READ_END], -1);
    close(fd[READ_END]);  // close parent's read end
}
