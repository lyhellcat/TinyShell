#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <unistd.h>    // pipe()
#include <errno.h>
#include <spawn.h>
#include <sys/wait.h>
#include <sys/signal.h>
#include <sys/stat.h>  // S_IRUSR
#include <fcntl.h>

#define PROMT_LEN       32
#define ARGS_NUM        32
#define COMMAND_LEN     1025
#define FILE_PATH_LEN   128
#define BUFFER_SIZE     128

typedef struct command {
    char *args[ARGS_NUM];
    int args_count;
    bool is_background;
    bool left_redirect;
    bool right_redirect;
    bool double_right_redirect;
    char redirect_file[FILE_PATH_LEN];

    struct command *next;
} command_t;

// for grep, compare string
int LineCompare(char *str1, char *str2) {
    int n = strlen(str1), m = strlen(str2);
    char temp[BUFFER_SIZE];
    for (int i = 0; i < n - m; i++) {
        memset(temp, 0, sizeof(temp));
        for (int j = 0; j < m; j++) {
            temp[j] = str1[i + j];
            if (strcmp(temp, str2) == 0) {  // matched
                return 1;
            }
        }
    }
    return 0;
}

void paerse(char *line, command_t *cmd) {
    char *token;
    const char s[] = " \t";
    token = strtok(line, s);

    int i;
    for (i = 0; token != NULL; i++) {
        if (*token == '&') {
            cmd->is_background = true;
            if ((token = strtok(NULL, s)) != NULL) {
                fprintf(stderr, "Error: \"&\" must be last token on command line\n");
                cmd->args[0] = NULL;
                continue;
            }
        } else if (strcmp(token, "<") == 0) {
            cmd->left_redirect = true;
            token = strtok(NULL, s);
            if (token == NULL) {
                fprintf(stderr, "Error: Missing filename for input redirection.\n");
                cmd->args[0] = NULL;
                continue;
            }
            strcpy(cmd->redirect_file, token);
        } else if (strcmp(token, ">") == 0) {
            cmd->right_redirect = true;
            token = strtok(NULL, s);
            if (token == NULL) {
                fprintf(stderr,
                        "Error: Missing filename for output redirection.\n");
                cmd->args[0] = NULL;
                continue;
            }
            strcpy(cmd->redirect_file, token);
        } else if (strcmp(token, ">>") == 0) {
            cmd->double_right_redirect = true;
            token = strtok(NULL, s);
            if (token == NULL) {
                fprintf(stderr, "Error: Missing filename for output redirection.\n");
                cmd->args[0] = NULL;
                continue;
            }
            strcpy(cmd->redirect_file, token);
        } else if (strcmp(token, "|") == 0) {
            cmd->next = malloc(sizeof(cmd));
            cmd->next->args_count = 0;
            // TODO: implement pipe
            while (token != NULL) {
                cmd->next->args[i] = strtok(NULL, s);
                cmd->next->args_count++;
            }
        }
        else {
            cmd->args[cmd->args_count++] = token;
        }
        token = strtok(NULL, s);
    }
    cmd->args[i] = NULL;
}

void ExcuteCommand(command_t cmd) {
    if (strcmp(cmd.args[0], "exit") == 0) {
        exit(0);
    } else if(strcmp(cmd.args[0], "info") == 0) {
        puts("COMP2211 Simplified Shell by sc19wc");
    } else if (strcmp(cmd.args[0], "pwd") == 0) {
        char cwd[BUFFER_SIZE];
        getcwd(cwd, sizeof(cwd));
        puts(cwd);
    } else if (strcmp(cmd.args[0], "cd") == 0) { // change work dir
        if (cmd.args[1][0] != '/') {
            char cwd[BUFFER_SIZE];
            getcwd(cwd, sizeof(cwd));
            strcat(cwd, "/");
            strcat(cwd, cmd.args[1]);
            chdir(cwd);
        } else {
            chdir(cmd.args[1]);
        }
    } else if (strcmp(cmd.args[0], "mygrep") == 0) {
        int cflag = 0;
        char *pattern_original = cmd.args[1];
        char *filename = cmd.args[2];
        if (strcmp(cmd.args[1], "-c") == 0) { // times of match
            cflag = 1;
            pattern_original = cmd.args[2];
            filename = cmd.args[3];
        }
        FILE *fp = fopen(filename, "r");
        if (fp == NULL) {
            perror("fopen");
            return;
        }
        char pattern[BUFFER_SIZE];
        for (int i = 0, j = 0; i < strlen(pattern_original); i++) {
            if (pattern_original[i] != '\"')
                pattern[j++] =  pattern_original[i];
        }
        // puts(pattern);
        char file_buffer[BUFFER_SIZE];
        int matched_cnt = 0;
        while (fgets(file_buffer, BUFFER_SIZE, fp)) {
            if (LineCompare(file_buffer, pattern)) {
                if (cflag == 0) printf("%s", file_buffer);
                matched_cnt++;
            }
        }
        if (cflag == 1) {
            printf("%d\n", matched_cnt);
        }
        fclose(fp);
    } else {
        if (strcmp(cmd.args[0], "ex") == 0) { // for ex command
            for (int i = 0; i < cmd.args_count - 1; i++) {
                strcpy(cmd.args[i], cmd.args[i + 1]);
            }
            cmd.args[cmd.args_count - 1] = NULL;
        }
        pid_t pid = fork();
        if (pid == 0) {
            // consider file redirect
            if (cmd.right_redirect) {
                // O_TRUNC, truncate the existing file so that its length is 0
                int fd = open(cmd.redirect_file, O_CREAT | O_WRONLY | O_TRUNC,
                              S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH);
                if (fd == -1) {
                    printf("Error: open(\"%s\"): %s\n", cmd.redirect_file, strerror(errno));
                    exit(EXIT_FAILURE);
                }
                // int stdout_fd = dup(STDOUT_FILENO);
                if (dup2(fd, STDOUT_FILENO) < 0) {
                    perror("dup2");
                    exit(EXIT_FAILURE);
                }
            }
            if (cmd.left_redirect) {
                int fd = open(cmd.redirect_file, O_CREAT | O_RDONLY,
                              S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH);
                if (fd == -1) {
                    printf("Error: open(\"%s\"): %s\n", cmd.redirect_file,
                           strerror(errno));
                    exit(EXIT_FAILURE);
                }
                if (dup2(fd, STDIN_FILENO) < 0) {
                    perror("dup2");
                    exit(EXIT_FAILURE);
                }
            }
            if (cmd.double_right_redirect) { // append redirect file
                int fd = open(cmd.redirect_file, O_CREAT | O_WRONLY | O_APPEND,
                              S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH);
                if (fd == -1) {
                    printf("Error: open(\"%s\"): %s\n", cmd.redirect_file,
                           strerror(errno));
                    exit(EXIT_FAILURE);
                }
                if (dup2(fd, STDOUT_FILENO) < 0) {
                    perror("dup2");
                    exit(EXIT_FAILURE);
                }
            }
            execvp(cmd.args[0], cmd.args);
            printf("%s: command not found\n", cmd.args[0]);
            exit(EXIT_FAILURE);
        } else {
            if (cmd.is_background) {
                signal(SIGCHLD, SIG_IGN);
                printf("%d\n", pid);
            } else {
                pid_t child_pid;
                for (;;) {
                    // waits for each child to exit
                    child_pid = wait(NULL);
                    if (child_pid == -1) {
                        if (errno == ECHILD) {
                            break;
                        } else {
                            perror("wait");
                            exit(1);
                        }
                    }
                }
            }
        }
    }
}

int main(int argc, char *argv[]) {
    signal(SIGINT, SIG_IGN);
    signal(SIGTSTP, SIG_IGN);

    char promt[PROMT_LEN] =
        "\033[0;32;32mmyshell$$ \033[m";  // set default promt
    bool show_promt = true;

    char cmd_line[COMMAND_LEN];

    for (;;) {
        command_t *cmd = calloc(1, sizeof(command_t));
        if (show_promt)
            printf("%s", promt);
        fflush(stdin);
        while (fgets(cmd_line, sizeof(cmd_line), stdin) == 0
            && errno == EINTR)
            ; // try again
        if (feof(stdin)) {
            free(cmd);
            break;
        }
        cmd_line[strlen(cmd_line) - 1] = '\0'; // replace '\n'
        paerse(cmd_line, cmd);
        if (cmd->args[0] == NULL) {
            free(cmd);
            continue;
        }
        ExcuteCommand(*cmd);
        free(cmd);
    }

    puts("\nThank you for using myshell!");

    return 0;
}
