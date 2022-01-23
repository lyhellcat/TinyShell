#define _GNU_SOURCE

#include <dirent.h>
#include <grp.h>
#include <pwd.h>
#include <regex.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <time.h>
#include <unistd.h>

int n, m;
char pattern[500];

int Index(const char* s, const char* p) {
    int i = 0, j = 0;
    while (i < n && j < m) {
        if (s[i] == p[j]) {
            i++, j++;
        } else {
            i = i - j + 1;
            j = 0;
        }
    }
    if (j >= m) {
        return i - m;
    } else
        return -1;
}

void list_dir(char* path) {
    DIR* pDir;
    struct dirent* pdirent;

    char childPath[512];

    if ((pDir = opendir(path)) != NULL) {
        while ((pdirent = readdir(pDir)) != NULL) {
            if (strcmp(pdirent->d_name, ".") == 0 ||
                strcmp(pdirent->d_name, "..") == 0)
                continue;
            n = strlen(pdirent->d_name);
            m = strlen(pattern);
            char output[1024];
            if (Index(pdirent->d_name, pattern) != -1) {
                sprintf(output, "%s/%s", path, pdirent->d_name);
                puts(output);
            }
            if (pdirent->d_type & DT_DIR) {
                sprintf(childPath, "%s/%s", path, pdirent->d_name);
                list_dir(childPath);
            }
        }
        closedir(pDir);
    }
}

int main(int argc, char* argv[]) {
    if (argc < 2) {
        printf("Usage: find [path...]");
        exit(0);
    }
    strcpy(pattern, argv[1]);
    list_dir(".");
}
