/* TODO:lab6-challenge */
#include <lib.h>
#define MAXDEPTH 50
int flag[256];
int file_cnt;
int dir_cnt;
void tree(char* path) {
    int r;
    struct Stat st;

    if ((r = stat(path, &st)) < 0) {
        user_panic("stat %s: %d", path, r);
    }

    if (!st.st_isdir) {
        printf("%s [error opening dir]\n", path);
        printf("0 directories, 0 files\n");
        exit();
    }

    printf("\033[33m%s\n\033[m", path);
    treedir(path, 0);
}

void treedir(char* path, int step) {
    int fd;
    int n;
    struct File f;

    if ((fd = open(path, O_RDONLY)) < 0) {
        return;
    }
    while ((n = readn(fd, &f, sizeof(f))) == sizeof(f)) {
        if (f.f_name[0]) {
            tree1(path, f.f_type == FTYPE_DIR, f.f_name, step + 1);
        }
    }
    if (n > 0) {
        user_panic("short read in directory %s", path);
    }
    if (n < 0) {
        user_panic("error reading directory %s: %d", path, n);
    }
}

void tree1(char* path, u_int isdir, char* name, int step) {
    char* sep;

    if (flag['d'] && !isdir) {
        return;
    }
    //
    if (step > MAXDEPTH) {
        debugf("tree is too deep");
        return;
    }
    //
    for (int i = 0; i < step - 1; i++) {
        printf("\033[34m    \033[m");
    }
    printf("\033[34m|-- \033[m");

    if (path[0] && path[strlen(path) - 1] != '/') {
        sep = "/";
    } else {
        sep = "";
    }

    if (flag['f'] && path) {
        printf("\033[34m%s%s\033[m", path, sep);
    }
    printf("\033[34m%s\n\033[m", name);

    if (isdir) {
        dir_cnt += 1;
        char newpath[256];
        strcpy(newpath, path);
        int namelen = strlen(name);
        int pathlen = strlen(path);
        if (strlen(sep) != 0) {
            newpath[pathlen] = '/';
            for (int i = 0; i < namelen; i++) {
                newpath[pathlen + i + 1] = name[i];
            }
            newpath[pathlen + namelen + 1] = 0;
            treedir(newpath, step);
        } else {
            for (int i = 0; i < namelen; i++) {
                newpath[pathlen + i] = name[i];
            }
            newpath[pathlen + namelen] = 0;
            treedir(newpath, step);
        }
    } else {
        file_cnt += 1;
    }
}
void usage(void) {
    printf("\033[31musage: tree [-adf] [directory...]\n\033[m");
    exit();
}
int main(int argc, char** argv) {
    int i;
    ARGBEGIN {
        default:
            usage();
        case 'a':
        case 'd':
        case 'f':
            flag[(u_char)ARGC()]++;
            break;
    }
    ARGEND
    int file[MAXDEPTH] = {0};
    if (argc == 0) {
        tree("./");
    } else {
        tree(argv[1]);
    }
    if (flag['d']) {
        printf("\033[32m\n%d directories\033[m\n", dir_cnt);
    } else {
        printf("\033[32m\n%d directories, %d files\033[m\n", dir_cnt, file_cnt);
    }
    printf("\n");
    return 0;
}
