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

#define PATH_BUFSIZE 1024
#define COMMAND_BUFSIZE 1024
#define MAXARG 20
#define TOKEN_BUFSIZE 64
#define TOKEN_DELIMITERS " \t\r\n\a"
#define BACKGROUND_EXECUTION 1
#define FOREGROUND_EXECUTION 0
#define PIPELINE_EXECUTION 2
#define ERROR_CMD 3
#define SWAP(a, b) (a ^= b ^= a ^= b)

struct command_segment {
    char *args[MAXARG];
    struct command_segment *next_cmd;
    pid_t pid;
    pid_t pgid;
    int r_redirection; // >, >>
    int l_redirection; // <, <<
    char redirect_to_file[PATH_BUFSIZE];
};

struct command {
    // 由于存在管道 因而有多个command_segment
    // 采用类似于LinkList的方式存储
    struct command_segment *root;
    int mode;   // 后台进程或是前台进程
};

char *pathvar[] = {"PATH=/bin:/usr/bin", "TERM=xterm", NULL};

void nyush_IO_redirecte(struct command* cmd) {
    if (cmd->root->r_redirection > 0) {
        int file_flag;
        if (cmd->root->r_redirection == 1)
            file_flag = O_WRONLY | O_TRUNC | O_CREAT;
        else
            file_flag = O_WRONLY | O_APPEND | O_CREAT;
        int wport = open(cmd->root->redirect_to_file, file_flag, 0777);
        dup2(wport, STDOUT_FILENO);
        close(wport);
    }
    if (cmd->root->l_redirection > 0) {  //<, <<
        int rport = open(cmd->root->redirect_to_file, O_RDONLY);
        dup2(rport, STDIN_FILENO);
        close(rport);
    }
}

void nyush_cd(char *path) {
   if (chdir(path) < 0)
        fprintf(stderr, "Error: invalid directory");
}

void nyush_fg(pid_t pid) {
    /* Implement fg command */
    setpgid(pid, pid);
    int status;
    if (tcsetpgrp(1, getpgid(pid)) == 0) {
        kill(pid, SIGCONT);        /* sucess */
        waitpid(pid, &status, WUNTRACED);
    }
    else
        printf("fg: job not found: %d\n", pid);
}

void nyush_bg(pid_t pid) {
    /* Implement bg command */
    int status;
    if (kill(pid, SIGCONT) < 0)
        printf("bg: job not found: %d\n", pid);
    else
        waitpid(pid, &status, WUNTRACED);
}

void nyush_kill(pid_t pid) {
    /* Kill child process */
   kill(pid, SIGKILL);
}

void sig_child(int signo) {
    if (signo != SIGCHLD) return;
    int status;
    wait(&status);
}

int nyushExecuteBuildinCommand(struct command_segment *segment){
    if (strcmp(segment->args[0], "cd") == 0) {
        if (segment->args[1] == NULL || segment->args[3] != NULL) {
            fprintf(stderr, "Error: invalid command");
            return 1;
        }
        nyush_cd(segment->args[1]);
        return 1;
    }
    else if (strcmp(segment->args[0], "exit") == 0) {
        return 0;
    }
    else if (strcmp(segment->args[0], "fg") == 0) {
        pid_t pid;
        pid = atoi(segment->args[1]);
        nyush_fg(pid);
        return 1;
    }
    else if (strcmp(segment->args[0], "bg") == 0) {
        pid_t pid;
        pid = atoi(segment->args[1]);
        nyush_bg(pid);
        return 1;
    }
    else if (strcmp(segment->args[0], "kill") == 0) {
        pid_t pid;
        pid = atoi(segment->args[1]);
        nyush_kill(pid);
        return 1;
    }
    else return -1;
}

int nyushExecuteCommandSegment(struct command_segment *segment, int in_fd, int out_fd, int mode, int pgid) {
    int status = 1;
    int isInCmd = nyushExecuteBuildinCommand(segment);
    /* Check if it's a build in command first */
    if (isInCmd == 0)  return -1;        /* exit cmd */
    else if (isInCmd > 0) return 1;      /* other buildin cmd */

    /* Fork a process and execute the program */
    pid_t childpid;
    childpid = fork();
    if (childpid < 0) {
        printf("fork failed\n");
    }
    else if (childpid == 0) {
        int mypid = getpid();
        signal(SIGINT, SIG_DFL);
        signal(SIGTSTP, SIG_DFL);
        signal(SIGCONT, SIG_DFL);

        dup2(in_fd, 0);
        dup2(out_fd, 1);
        if(in_fd != 0){
            close(in_fd);
        }
        if(out_fd != 1){
            close(out_fd);
        }
        execvpe(segment->args[0], segment->args, pathvar);
        if (errno == ENOENT) {
            fprintf(stderr, "Can not find command: %s\n", segment->args[0]);
        }
    }
    else {
        // printf("parent process\n");
        signal(SIGINT, SIG_IGN);
        signal(SIGTSTP, SIG_IGN);
        signal(SIGCONT, SIG_DFL);
        if (mode == BACKGROUND_EXECUTION)
            signal(SIGCHLD, SIG_IGN);
        else
            waitpid(childpid, &status, WUNTRACED);
        if(in_fd != 0){
            close(in_fd);
        }
        if(out_fd != 1){
            close(out_fd);
        }
    }
    return 1;
}

int nyushExecuteCommand(struct command *command) {
    // blank line
    int sfd = dup(STDOUT_FILENO);
    if (command->root->args[0] == "BLANK")
        return 1;
    if (command->root->args[0] == "ERROR_CMD") {
        puts("Error: invalid command");
        return 3;
    }
    // IO Redirect
    if (command->root->l_redirection || command->root->r_redirection) {
        nyush_IO_redirecte(command);
    }

    int status = 1;
    struct command_segment* cur;
    struct command_segment* pfree;

    int temp_fd = 0;
    for (cur = command->root; cur != NULL; cur = cur->next_cmd) {
        if (cur->next_cmd) {
            int fd[2];
            pipe(fd);
            status = nyushExecuteCommandSegment(cur, temp_fd, fd[1], command->mode, 0);
            temp_fd = fd[0];
        }
        else {
            status = nyushExecuteCommandSegment(cur, temp_fd, 1, command->mode, 0);
        }
    }
    // IO Redirect
    if (command->root->l_redirection || command->root->r_redirection) {
        dup2(sfd, STDOUT_FILENO);
        close(sfd);
    }

    /* print the decompoted command and free the space */
    cur = command->root;
    pfree = cur;
    while (cur != NULL) {
        cur = cur->next_cmd;
        free(pfree);
        pfree = cur;
    }
    cur = NULL;
    pfree = NULL;
    free(command);
    command = NULL;

    return status;
}

struct command* nyushParseCommand(char *line) {
    struct command* command = malloc(sizeof(struct command));
    command->root = malloc(sizeof(struct command_segment));
    struct command_segment* cur;
    struct command_segment* pnew;
    cur = command->root;
    char back[PATH_BUFSIZE];
    strcpy(back, line);
    // 判断是否为空行
    int is_blank = 1;
    for (int i = 0; i < strlen(line); i++) {
        if (line[i] != ' ' && line[i] != '\r' && line[i] != '\n')
            is_blank = 0;
    }
    if (is_blank) {
        command->root->args[0] = "BLANK";
        return command;
    }

    // 是否后台程序
    char* pStart = line;
    int count = 0;
    while ((*pStart != '\n') && (*pStart != '\0')) {
        if (*pStart == '&') {
            count = 1;
            *pStart = '\0';
            break;
        }
        pStart++;
    }
    command->mode = count;

    char *res = line;
    char *temp;
    int i = 0;

    temp = strsep(&res, "|<>");

    for (i = 0; i < MAXARG - 1 && (cur->args[i] = strtok(temp, TOKEN_DELIMITERS)) != NULL; i++) {
        temp  = NULL;
    }

    cur->args[i] = NULL;
    int check_delim = 0;
    while ((temp = strsep(&res, "|<>")) != NULL) {
        check_delim++;
        pnew = malloc(sizeof(struct command_segment));
        cur->next_cmd = pnew;
        cur = pnew;
        for (i = 0; i < MAXARG - 1 && (cur->args[i] = strtok(temp, TOKEN_DELIMITERS)) != NULL; i++) {
            temp = NULL;
            check_delim = 0;
        }
        cur->args[i] = NULL;
    }

    if (check_delim || command->root->args[0] == NULL) {
        command->root->args[0] = "ERROR_CMD";
    }
    cur->next_cmd = NULL;

    if (command->root->args[0] != "ERROR_CMD") {
        res = back;

        for (int i = 0; i < strlen(res); i++) {
            if (res[i] == '<') {
                command->root->l_redirection = 1;
                if (res[i] + 1 == '<')
                    command->root->l_redirection++;
                char *p;
                strtok(res, ">");
                p = strtok(NULL, TOKEN_DELIMITERS);
                strcpy(command->root->redirect_to_file, p);
                command->root->next_cmd = NULL;
            }
            if (res[i] == '>') {
                command->root->r_redirection = 1;
                if (res[i] + 1 == '>')
                    command->root->r_redirection++;
                char *p;
                strtok(res, ">");
                p = strtok(NULL, TOKEN_DELIMITERS);
                strcpy(command->root->redirect_to_file, p);
                printf("Redirect to: %s\n", command->root->redirect_to_file);
                command->root->next_cmd = NULL;
            }
        }

    }

    return command;
}

// 获取当前工作目录
void GetPwd(char *cur_dir_name) {
    char buffer[1024];
    getcwd(buffer, 1024);

    int cnt = 0;
    int i = strlen(buffer) - 1;
    while (buffer[i] != '/') {
        cur_dir_name[cnt++] = buffer[i--];
    }
    cur_dir_name[cnt] = '\0';
    for (int i = 0; i < strlen(cur_dir_name) / 2; i++) {
        SWAP(cur_dir_name[i], cur_dir_name[strlen(cur_dir_name) - 1 - i]);
    }
}

// 显示Promt
void nyushPrintPromt() {
    char cur_dir_name[COMMAND_BUFSIZE];
    GetPwd(cur_dir_name);
    fprintf(stdout, "[nyush %s]$ ", cur_dir_name);
}

void nyush() {
    char line[COMMAND_BUFSIZE + 10];
    struct command *command;
    int status = 1;

    do {
        nyushPrintPromt();
        fflush(stdin);
        fgets(line, COMMAND_BUFSIZE, stdin);
        if (strlen(line) == 0) {
            continue;
        }
        command = nyushParseCommand(line);
        status = nyushExecuteCommand(command);
    } while (status >= 0);
}

int main(int argc, char **argv) {
    nyush();
    return EXIT_SUCCESS;
}
