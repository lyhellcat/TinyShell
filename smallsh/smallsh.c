#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <unistd.h>
#include <linux/limits.h>

#define MAXARGS 512
#define MAXLINE 2048

char prompt[] = ": ";

typedef struct command {
    char *argv[MAXARGS];
    int argc;
    bool is_background;
    bool left_redirect;
    bool right_redirect;
    char input_file[PATH_MAX];
    char output_file[PATH_MAX];
} command_t;

void parse(command_t *cmd, char *line) {
    memset(cmd, 0, sizeof(command_t));
    cmd->argc = 0;
    char *token;
    const char delim[] = " \t";
    token = strtok(line, delim);

    while (token != NULL) {
        printf("===== %s ======\n", token);
        if (*token == '<') {
            cmd->left_redirect = true;
            token = strtok(NULL, delim);
            strcpy(cmd->input_file, token);
        } else if (*token == '>') {
            cmd->right_redirect = true;
            token = strtok(NULL, delim);
            strcpy(cmd->output_file, token);
        } else {
            cmd->argv[cmd->argc++] = token;
        }
        token = strtok(NULL, delim);
    }
    if ((cmd->is_background = (*(cmd->argv[cmd->argc - 1]) == '&'))
         != false) {
        cmd->argv[--(cmd->argc)] = NULL;
    }
}

void PrintCommand(command_t cmd) {
    for (int i = 0; i < cmd.argc; i++) {
        puts(cmd.argv[i]);
    }
    printf("%s ", cmd.is_background ? "bg" : "no bg");
    printf("%s ", cmd.left_redirect ? "IS left" : "no l");
    printf("%s ", cmd.right_redirect ? "Is right " : "no righnt");
    printf("%s ", cmd.input_file);
    printf("%s ", cmd.output_file);
    puts("");
}

int main() {
    static char line[MAXLINE + 1];

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
        command_t cmd;
        parse(&cmd, line);
        PrintCommand(cmd);

        fflush(stdout);
        fflush(stdout);
    }
    return 0;
}
