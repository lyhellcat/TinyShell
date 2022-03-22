#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <signal.h>
#include <unistd.h>
#include <linux/limits.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <errno.h>

#define MAXARGS 512
#define MAXLINE 2048

// Define global variables
char prompt[] = ": ";
int status = 0;
volatile sig_atomic_t isBackgroundJobFinished = 0;
volatile bool isForegroundOnly = false;
volatile bool isCTRL_Z = false;
volatile pid_t Finish_pid = 0;

typedef void handler_t(int);

typedef struct command {
    char *argv[MAXARGS];
    int argc;
    bool is_background;
    bool do_input_redirect;
    bool do_output_redirect;
    char input_file[PATH_MAX];
    char output_file[PATH_MAX];
} command_t;

char expansion[MAXLINE];

void parse(command_t *cmd, char *line) {
    line[strlen(line) - 1] = '\0';      // Replace '\n'
    memset(cmd, 0, sizeof(command_t));
    cmd->argc = 0;
    char *token;
    const char delim[] = " \t";
    token = strtok(line, delim);

    while (token != NULL) {
        if (*token == '<') {
            cmd->do_input_redirect = true;
            token = strtok(NULL, delim);
            strcpy(cmd->input_file, token);
        } else if (*token == '>') {
            cmd->do_output_redirect = true;
            token = strtok(NULL, delim);
            strcpy(cmd->output_file, token);
        } else if (strstr(token, "$$") != NULL) {
            char *p = strstr(token, "$$");
            *p = '\0';
            sprintf(expansion, "%s%d%s", token, getpid(), p + 2);
            cmd->argv[cmd->argc++] = expansion;
        } else {
            cmd->argv[cmd->argc++] = token;
        }
        token = strtok(NULL, delim);
    }
    if (cmd->argc && (cmd->is_background = (*(cmd->argv[cmd->argc - 1]) == '&'))
         != false) {
        cmd->argv[--(cmd->argc)] = NULL;
        if (isForegroundOnly) {
            cmd->is_background = false;
        }
    }
}

/**
 * Signal - wrapper for the sigaction function
 */
handler_t *Signal(int signum, handler_t *handler) {
    struct sigaction action, old_action;

    action.sa_handler = handler;
    sigemptyset(&action.sa_mask); /* block sigs of type being handled */
    action.sa_flags = SA_RESTART; /* restart syscalls if possible */

    if (sigaction(signum, &action, &old_action) < 0)
        perror("Signal error");
    return (old_action.sa_handler);
}

/**
 * @brief Execute builtin command, if is buildtin cmd return true
 *        else return false
 * @param cmd
 * @return true
 * @return false
 */
bool builtin_cmd(command_t* cmd) {
    if (strcmp(cmd->argv[0], "exit") == 0) {
        exit(0);
    } else if (strcmp(cmd->argv[0], "cd") == 0) {
        if (cmd->argc == 1) {
            char *homedir = getenv("HOME");
            chdir(homedir);
        } else {
            chdir(cmd->argv[1]);
        }
        return 1;
    } else if (strcmp(cmd->argv[0], "status") == 0) {
        if (WIFEXITED(status)) {
            fprintf(stdout, "exit value %d\n", WEXITSTATUS(status));
        } else if (WIFSIGNALED(status)) {
            fprintf(stdout, "terminated by signal %d\n", WTERMSIG(status));
        }
        return true;
    }
    return false;
}

void EvaluateCommand(command_t* cmd) {
    pid_t pid;

    if (cmd->argv[0] == NULL || *(cmd->argv[0]) == '#' || *(cmd->argv[0]) == '\n') {
        return;
    }
    if (builtin_cmd(cmd))
        return;
    if ((pid = fork()) == 0) {
        // Do input & output redirection
        if (cmd->do_input_redirect) {
            int fd;
            if ((fd = open(cmd->input_file, O_RDONLY)) < 0) {
                fprintf(stdout, "cannot open %s for input\n", cmd->input_file);
                exit(1);
            }
            dup2(fd, STDIN_FILENO);
        }
        if (cmd->do_output_redirect) {
            int fd;
            if ((fd = open(cmd->output_file,  O_CREAT | O_TRUNC | O_RDWR,
                    S_IRUSR | S_IWUSR)) < 0) {
                fprintf(stdout, "cannot open %s for output\n", cmd->output_file);
                exit(1);
            }
            dup2(fd, STDOUT_FILENO);
        }
        if (cmd->do_input_redirect == false && cmd->is_background) {
            int fd;
            if ((fd = open("/dev/null", O_RDONLY)) < 0) {
                perror("open error");
                exit(1);
            }
            dup2(fd, STDIN_FILENO);
        }
        if (cmd->do_output_redirect == false && cmd->is_background) {
            int fd;
            if ((fd = open("/dev/null",  O_CREAT | O_TRUNC | O_RDWR,
                    S_IRUSR | S_IWUSR)) < 0) {
                perror("open error");
                exit(1);
            }
            dup2(fd, STDOUT_FILENO);
        }
        if (cmd->is_background) {
            // Children running as background process ignore CTRL-C
            Signal(SIGINT, SIG_IGN);
        } else {
            Signal(SIGINT, SIG_DFL);
        }
        if (execvp(cmd->argv[0], cmd->argv) < 0) {
            fprintf(stdout, "%s: no such file or directory\n", cmd->argv[0]);
            exit(1);
        }
    }
    if (cmd->is_background) {
        fprintf(stdout, "background pid is %d\n", pid);
    } else {
        if (waitpid(pid, &status, WUNTRACED | WCONTINUED) < 0 && errno != ECHILD) {
            perror("waitpid error");
            exit(1);
        }
        if (WIFSIGNALED(status) && !isBackgroundJobFinished) {
            fprintf(stdout, "terminated by signal %d\n", WTERMSIG(status));
        }
    }
}

/**
 * sigtstp_handler - handler CTRL-z
 */
void sigstp_handler(int sig) {
    isCTRL_Z = true;
    isForegroundOnly = !isForegroundOnly;
}

/**
 * sigchld_handler - The kernel sends a SIGCHLD to the shell whenever
 *     a child job terminates (becomes a zombie), or stops because it
 *     received a SIGSTOP or SIGTSTP signal. The handler reaps all
 *     available zombie children, but doesn't wait for any other
 *     currently running children to terminate.
 */
void sigchld_handler(int sig) {
    pid_t pid;
    while ((pid = waitpid(-1, &status, WNOHANG | WUNTRACED)) > 0) {
        Finish_pid = pid;
        isBackgroundJobFinished = 1;
    }
}

int main() {
    static char line[MAXLINE + 1];

    Signal(SIGINT, SIG_IGN);
    Signal(SIGTSTP, sigstp_handler);
    Signal(SIGCHLD, sigchld_handler);

    while (1) {
        fprintf(stdout, "%s", prompt);
        fflush(stdout);

        if ((fgets(line, MAXLINE, stdin) == NULL) && ferror(stdin)) {
            perror("fgets error");
            exit(1);
        }
        if (feof(stdin)) {
            fflush(stdout);
            exit(0);
        }

        while (isCTRL_Z) {
            isCTRL_Z = false;
            if (isForegroundOnly) {
                fprintf(stdout,
                        "Entering foreground-only mode (& is now ignored)\n");
            } else {
                fprintf(stdout, "Exiting foreground-only mode\n");
            }
        }
        fflush(stdout);

        command_t cmd;
        parse(&cmd, line);
        EvaluateCommand(&cmd);
        fflush(stdout);

        while (isBackgroundJobFinished) {
            isBackgroundJobFinished = 0;
            if (WIFSIGNALED(status)) {
                fprintf(stdout, "background pid %d is done: terminated by signal %d\n",
                        Finish_pid, WTERMSIG(status));
            } else if (WIFEXITED(status)) {
                fprintf(stdout, "background pid %d is done: exit value %d\n",
                        Finish_pid, WEXITSTATUS(status));
            }
        }
        fflush(stdout);
        fflush(stdout);
    }
    return 0;
}
