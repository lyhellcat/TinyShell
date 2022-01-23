#define _GNU_SOURCE

#include <dirent.h>
#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

void PrintDirentStruct(char *direntName, int level) {
    struct dirent *d[1024];
    int cnt = 0;
    DIR *p_dir = NULL;
    struct dirent *p_dirent = NULL;
    p_dir = opendir(direntName);

    while ((p_dirent = readdir(p_dir)) != NULL) {
        d[cnt++] = p_dirent;
    }
    struct dirent *temp;
    for (int i = 0; i < cnt - 1; ++i) {
        for (int j = i + 1; j < cnt; ++j) {
            if (strcasecmp(d[i]->d_name, d[j]->d_name) > 0) {
                temp = d[i];
                d[i] = d[j];
                d[j] = temp;
            }
        }
    }

    for (int i = 0; i < cnt; i++) {
        p_dirent = d[i];
        char *backupDirName = NULL;
        if (p_dirent->d_name[0] == '.') {
            continue;
        }
        for (int j = 0; j < level; j++) {
            printf("  ");
        }
        printf("- ");
        printf("%s\n", p_dirent->d_name);

        if (p_dirent->d_type == DT_DIR) {
            int curDirentNameLen = strlen(direntName) + 1;

            backupDirName = (char *)malloc(curDirentNameLen);
            memset(backupDirName, 0, curDirentNameLen);
            memcpy(backupDirName, direntName, curDirentNameLen);

            strcat(direntName, "/");
            strcat(direntName, p_dirent->d_name);
            PrintDirentStruct(direntName, level + 1);

            memcpy(direntName, backupDirName, curDirentNameLen);
            free(backupDirName);
            backupDirName = NULL;
        }
    }
    closedir(p_dir);
}

int main(int argc, char *argv[]) {
    char direntName[256];
    memset(direntName, 0, sizeof(direntName));

    if (argc == 1) {
        direntName[0] = '.';
    }

    printf("%s\n", direntName);
    PrintDirentStruct(direntName, 0);
    return 0;
}
