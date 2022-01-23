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

#define COMMAND_BUFSIZE 500
#define ARGS_NUM 10
#define SUCESS_EXECUTE 1
#define COMMAND_EXIT -1
#define UNKNOW_COMMAND -3
#define BLANK_LINE 1

typedef struct _command {
    char *args[ARGS_NUM];
    int args_count;
    int is_background;
} command;

// pid to jobs id
int bg_id[100010], fg_id[100010];
// jobs id to pid
int bg_jobs_id[110], fg_jobs_id[110];
int bg_cnt, fg_cnt;

command* ParseCommand(char *line) {
    command* cmd = malloc(sizeof(command));
    char *token;
    const char s[] = " ";
    token = strtok(line, s);
    int i;
    cmd->is_background = 0;
    cmd->args_count = 0;
    for (i = 0; token != NULL; i++) {
        if (*token == '&') {
            cmd->is_background = 1;
        } else {
            cmd->args[i] = token;
        }
        token = strtok(NULL, s);
    }
    cmd->args_count = i;
    cmd->args[i] = NULL;
    return cmd;
}

int is_ctrl_c_pressed;

void sig_handler_ctrl_c(int sig) {
    if (sig == SIGINT) {
        is_ctrl_c_pressed = 1;
    }
}

int ExecuteCommand(command* cmd) {
    // build in command
    if (cmd->args[0] == NULL) {
        return BLANK_LINE;
    }
    if (strcmp(cmd->args[0], "exit") == 0) {
        return COMMAND_EXIT;
    } else if (strcmp(cmd->args[0], "cd") == 0) {
        if (cmd->args_count == 1) {
            // no path is given
            char *pathvar = getenv("HOME");
            chdir(pathvar);
        } else {
            chdir(cmd->args[1]);
            if (cmd->args[1][0] != '/') {
                char *pathvar = getenv("PWD");
                strcat(pathvar, "/");
                strcat(pathvar, cmd->args[1]);
                strcpy(cmd->args[1], pathvar);
            }
            puts(cmd->args[1]);
            // update the environment variable PWD
            setenv("PWD", cmd->args[1], 1);
        }
    } else if (strcmp(cmd->args[0], "kill") == 0) {

    } else if (strcmp(cmd->args[0], "fg") == 0) {

    } else if (strcmp(cmd->args[0], "jobs") == 0) {
        for (int i = 0; i < fg_cnt; i++) {
            printf("[%d] %d\n", fg_cnt + 1, fg_jobs_id[i]);
        }
    } else {
        pid_t child_pid;
        child_pid = fork();
        if (child_pid < 0) {
            perror("fork() failed: ");
            return COMMAND_EXIT;
        } else if (child_pid == 0) {
            signal(SIGINT, SIG_DFL);
            signal(SIGTSTP, SIG_DFL);
            signal(SIGCONT, SIG_DFL);

            int is_path = 0;
            for (int i = 0; i < strlen(cmd->args[0]); i++) {
                if (cmd->args[0][i] == '/') {
                    is_path = 1;
                    break;
                }
            }
            if (is_path) {
                execvp(cmd->args[0], cmd->args);
                if (errno == ENOENT) {
                    fprintf(stderr, "%s: No such file or directory\n", cmd->args[0]);
                }
            } else {
                char* pathvar[] = {"PATH=/usr/bin:/usr/bin", "TERM=xterm",
                NULL};
                execvpe(cmd->args[0], cmd->args, pathvar);
                if (errno == ENOENT) {
                    fprintf(stderr, "%s: Can not find command\n",
                    cmd->args[0]);
                }
            }
        } else {
            // parent process
            signal(SIGINT, sig_handler_ctrl_c);
            signal(SIGTSTP, SIG_IGN);
            signal(SIGCONT, SIG_DFL);
            if (cmd->is_background) {
                fg_id[child_pid] = ++fg_cnt;
                fg_jobs_id[fg_cnt - 1] = child_pid;
                printf("[%d] %d\n", fg_id[child_pid], child_pid);
                signal(SIGCHLD, SIG_IGN);
            }
            else {
                bg_id[child_pid] = ++bg_cnt;
                waitpid(child_pid, NULL, WUNTRACED);
                if (is_ctrl_c_pressed) {
                    printf("\n[%d] %d terminated by signal 2\n",
                            bg_id[child_pid], child_pid);
                    is_ctrl_c_pressed = 0;
                }
            }
        }
    }
    return SUCESS_EXECUTE;
}


int main() {
    signal(SIGINT, SIG_IGN);
    signal(SIGTSTP, SIG_IGN);
    char line[COMMAND_BUFSIZE + 10];
    int status = SUCESS_EXECUTE;
    do {
        printf("> ");
        fflush(stdin);
        fgets(line, COMMAND_BUFSIZE, stdin);
        line[strlen(line) - 1] = '\0'; // replace '\n'
        command* cmd = ParseCommand(line);
        status = ExecuteCommand(cmd);
        free(cmd);
    } while(status >= 0);
}
