#include <ctype.h>
#include <errno.h>
#include <fcntl.h>
#include <signal.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <termios.h>
#include <unistd.h>

#include "tokenizer.h"

#define IO_INPUT  1
#define IO_OUTPUT 2

/* Convenience macro to silence compiler warnings about unused function
 * parameters. */
#define unused __attribute__((unused))

/* Whether the shell is connected to an actual terminal or not. */
bool shell_is_interactive;

/* File descriptor for the shell input */
int shell_terminal;

/* Terminal mode settings for the shell */
struct termios shell_tmodes;

/* Process group id for the shell */
pid_t shell_pgid;

int cmd_exit(struct tokens* tokens);
int cmd_help(struct tokens* tokens);
int cmd_pwd(struct tokens* tokens);
int cmd_cd(struct tokens* tokens);

void execute(struct tokens* tokens);

/* Built-in command functions take token array (see parse.h) and return int */
typedef int cmd_fun_t(struct tokens* tokens);

/* Built-in command struct and lookup table */
typedef struct fun_desc {
    cmd_fun_t* fun;
    char* cmd;
    char* doc;
} fun_desc_t;

// Input arguments type
typedef struct parsed_args {
    char ***args_arr;
    size_t *args_len;
    int *io_idxs;
    int *io_tags;
    size_t proc_num;
} parsed_args_t;

fun_desc_t cmd_table[] = {
    {cmd_help, "?", "show this help menu"},
    {cmd_exit, "exit", "exit the command shell"},
    {cmd_pwd, "pwd", "print the current working directory to standard output"},
    {cmd_cd, "cd", "changes current working directory to token"}
};

/* Prints a helpful description for the given command */
int cmd_help(unused struct tokens* tokens) {
    for (unsigned int i = 0; i < sizeof(cmd_table) / sizeof(fun_desc_t); i++)
        printf("%s - %s\n", cmd_table[i].cmd, cmd_table[i].doc);
    return 1;
}

/* Exits this shell */
int cmd_exit(unused struct tokens* tokens) {
    exit(0);
}

/* Print the current working directory */
int cmd_pwd(unused struct tokens* tokens) {
    char buf[BUFSIZ];
    if (getcwd(buf, BUFSIZ - 1) == NULL) {
        perror("getcwd");
        exit(EXIT_FAILURE);
    }
    fprintf(stdout, "%s\n", buf);
    return 1;
}

/* changes current working directory to token */
int cmd_cd(struct tokens* tokens) {
    char *path = tokens_get_token(tokens, 1);
    chdir(path);
    return 1;
}

/* Looks up the built-in command, if it exists. */
int lookup(char cmd[]) {
    for (unsigned int i = 0; i < sizeof(cmd_table) / sizeof(fun_desc_t); i++)
        if (cmd && (strcmp(cmd_table[i].cmd, cmd) == 0))
            return i;
    return -1;
}

/* Get proc nums for command */
size_t tokens_get_proc_num(struct tokens *tokens) {
    size_t tokens_len = tokens_get_length(tokens);
    size_t proc_num = 1;
    // If the tokens contain "|", it means there are multiple processes
    for (size_t i = 0; i < tokens_len; i++) {
        if (strcmp(tokens_get_token(tokens, i), "|") == 0) {
            proc_num++;
        }
    }
    return proc_num;
}

/* Parse shell commands and convert to shell args */
parsed_args_t* parse_args(struct tokens *tokens) {
    size_t token_len = tokens_get_length(tokens);
    size_t proc_num = tokens_get_proc_num(tokens);

    parsed_args_t *parsed_args = calloc(1, sizeof(parsed_args_t));
    parsed_args->args_arr = calloc(proc_num, sizeof(char **));
    parsed_args->args_len = calloc(proc_num, sizeof(size_t) * proc_num);
    parsed_args->io_idxs = calloc(proc_num, sizeof(int) * proc_num);
    parsed_args->io_tags = calloc(proc_num, sizeof(int) * proc_num);

    size_t idx = 0;
    for (size_t i = 0; i != proc_num; i++) {
        size_t j;
        for (j = idx; j != token_len; j++) {
            parsed_args->args_len[i]++;
            if (strcmp(tokens_get_token(tokens, j), "|") == 0)
                break;
        }
        parsed_args->args_arr[i] = calloc(1, parsed_args->args_len[i] * sizeof(char *));
        for (j = idx; j != token_len; j++) {
            char* token = tokens_get_token(tokens, j);
            if (strcmp(token, "|") == 0) {
                break;
            } else {
                parsed_args->args_arr[i][j - idx] = token;
            }
            if (strcmp(token, ">") == 0) {
                parsed_args->io_tags[i] = IO_OUTPUT;
                parsed_args->io_idxs[i] = j - idx;
            } else if (strcmp(token, "<") == 0) {
                parsed_args->io_tags[i] = IO_INPUT;
                parsed_args->io_idxs[i] = j - idx;
                printf("%d\n", parsed_args->io_idxs[i]);
            }
        }
        idx = j + 1;
    }
    return parsed_args;
}

/* Intialization procedures for this shell */
void init_shell() {
    /* Our shell is connected to standard input. */
    shell_terminal = STDIN_FILENO;

    /* Check if we are running interactively */
    shell_is_interactive = isatty(shell_terminal);

    if (shell_is_interactive) {
        /* If the shell is not currently in the foreground, we must pause the
         * shell until it becomes a foreground process. We use SIGTTIN to pause
         * the shell. When the shell gets moved to the
         * foreground, we'll receive a SIGCONT. */
        while (tcgetpgrp(shell_terminal) != (shell_pgid = getpgrp()))
            kill(-shell_pgid, SIGTTIN);

        /* Saves the shell's process id */
        shell_pgid = getpid();

        /* Take control of the terminal */
        tcsetpgrp(shell_terminal, shell_pgid);

        /* Save the current termios to a variable, so it can be restored later.
         */
        tcgetattr(shell_terminal, &shell_tmodes);
    }
}

void execute(struct tokens* tokens) {
    parse_args(tokens);
}

int main(unused int argc, unused char* argv[]) {
    init_shell();

    static char line[4096];
    int line_num = 0;

    /* Please only print shell prompts when standard input is not a tty */
    if (shell_is_interactive) fprintf(stdout, "%d: ", line_num);

    while (fgets(line, 4096, stdin)) {
        /* Split our line into words. */
        struct tokens* tokens = tokenize(line);

        /* Find which built-in function to run. */
        int fundex = lookup(tokens_get_token(tokens, 0));

        if (fundex >= 0) {
            cmd_table[fundex].fun(tokens);
        } else {
            // Run commands as program
            pid_t pid;
            if (tokens_get_length(tokens) != 0) {
                execute(tokens);
                // if ((pid = fork()) == 0) {
                //     execute(tokens);
                // } else {
                //     wait(NULL);
                // }
                fprintf(stdout, "This shell doesn't know how to run programs.\n");
            }
        }

        if (shell_is_interactive)
            /* Please only print shell prompts when standard input is not a tty
             */
            fprintf(stdout, "%d: ", ++line_num);

        /* Clean up memory */
        tokens_destroy(tokens);
    }

    return 0;
}
