#include <dirent.h>
#include <grp.h>
#include <pwd.h>
#include <stdio.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include <time.h>

char filename[1024][1024];
int cnt;

void mode_to_letters(int mode, char str[]) {
    strcpy(str, "----------");
    if (S_ISDIR(mode)) {
        str[0] = 'd';
    }
    if (S_ISCHR(mode)) {
        str[0] = 'c';
    }
    if (S_ISBLK(mode)) {
        str[0] = 'b';
    }
    if ((mode & S_IRUSR)) {
        str[1] = 'r';
    }
    if ((mode & S_IWUSR)) {
        str[2] = 'w';
    }
    if ((mode & S_IXUSR)) {
        str[3] = 'x';
    }
    if ((mode & S_IRGRP)) {
        str[4] = 'r';
    }
    if ((mode & S_IWGRP)) {
        str[5] = 'w';
    }
    if ((mode & S_IXGRP)) {
        str[6] = 'x';
    }
    if ((mode & S_IROTH)) {
        str[7] = 'r';
    }
    if ((mode & S_IWOTH)) {
        str[8] = 'w';
    }
    if ((mode & S_IXOTH)) {
        str[9] = 'x';
    }
}

char *uid_to_name(uid_t uid) {
    struct passwd *getpwuid(), *pw_ptr;
    static char numstr[10];

    if ((pw_ptr = getpwuid(uid)) == NULL) {
        sprintf(numstr, "%d", uid);
        return numstr;
    } else {
        return pw_ptr->pw_name;
    }
}

char *gid_to_name(gid_t gid) {
    struct group *getgrgid(), *grp_ptr;
    static char numstr[10];

    if ((grp_ptr = getgrgid(gid)) == NULL) {
        sprintf(numstr, "%d", gid);
        return numstr;
    } else {
        return grp_ptr->gr_name;
    }
}

void show_file_info(char *filename, struct stat *info_p) {
    char modestr[11];

    mode_to_letters(info_p->st_mode, modestr);

    printf("%s", modestr);
    printf(" %-8s", uid_to_name(info_p->st_uid));
    printf(" %-8s", gid_to_name(info_p->st_gid));
    printf(" %8ld", (long)info_p->st_size);
    printf(" %.12s", 4 + ctime(&info_p->st_mtime));
    printf(" %s\n", filename);
}

void ls(char *dirpath) {
    DIR *pDir;
    struct dirent *pdirent;

    if ((pDir = opendir(dirpath)) == NULL) {
        fprintf(stderr, "ls: Can not open %s\n", dirpath);
    } else {
        while ((pdirent = readdir(pDir)) != NULL) {
            if (strcmp(pdirent->d_name, ".") == 0 ||
                strcmp(pdirent->d_name, "..") == 0) {
                continue;
            }
            strcpy(filename[cnt], pdirent->d_name);
            cnt++;
        }
        closedir(pDir);
    }
}

void sort_filename() {
    char temp[1024];
    for (int i = 0; i < cnt - 1; ++i) {
        for (int j = i + 1; j < cnt; ++j) {
            if (strcasecmp(filename[i], filename[j]) > 0) {
                strcpy(temp, filename[i]);
                strcpy(filename[i], filename[j]);
                strcpy(filename[j], temp);
            }
        }
    }
}

int main(int argc, char *argv[]) {
    ls(".");
    sort_filename();
    if (argc == 1) // ls with no command-line options
    {
        for (int i = 0; i < cnt; i++)
            puts(filename[i]);
    } else if (strcmp(argv[0], "-l")) {
        struct stat info;
        for (int i = 0; i < cnt; i++) {
            char complete_d_name[200];
            strcpy(complete_d_name, ".");
            strcat(complete_d_name, "/");
            strcat(complete_d_name, filename[i]);
            struct stat info;
            if (stat(complete_d_name, &info) != -1) {
                show_file_info(filename[i], &info);
            }
        }
    }
    return 0;
}
