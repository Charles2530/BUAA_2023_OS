#include <args.h>
#include <lib.h>

#define WHITESPACE " \t\r\n"
#define SYMBOLS "<|>&;()"
/* TODO:lab6-challenge */
void chpwd(int argc, char** argv) {
    int r;
    if (argc == 1) {
        if ((r = chdir("/")) < 0) {
            printf("cd failed: %d\n", r);
            exit();
        }
        printf("/\n");
    } else {
        if (argv[1][0] == '/') {
            if ((r = chdir(argv[1])) < 0) {
                printf("cd failed: %d\n", r);
                exit();
            }
        } else {
            // Get curent direct path.
            char path[128];
            if ((r = getcwd(path)) < 0) {
                printf("cd failed: %d\n", r);
                exit();
            }
            if (strcmp(argv[1], "..") == 0) {
                int len = strlen(path);
                while (path[len - 1] != '/') {
                    path[len - 1] = '\0';
                    len--;
                }
                if (strcmp(path, "/") != 0) {
                    path[len - 1] = '\0';
                }
            } else {
                // Parse the target indirect path into direct.
                pathcat(path, argv[1]);
            }

            // Confirm the path exists and is a directory.
            if ((r = open(path, O_RDONLY)) < 0) {
                printf("path %s doesn't exist: %d\n", path, r);
                exit();
            }
            close(r);
            struct Stat st;
            if ((r = stat(path, &st)) < 0) {
                user_panic("stat %s: %d", path, r);
            }
            if (!st.st_isdir) {
                printf("path %s is not a directory\n", path);
                exit();
            }

            if ((r = chdir(path)) < 0) {
                printf("cd failed: %d\n", r);
                exit();
            }
        }
    }
    return;
}
void do_incmd(int argc, char** argv) {
    if (strcmp(argv[0], "cd") == 0 || strcmp(argv[0], "cd.b") == 0) {
        chpwd(argc, argv);
    } else {
        /* TODO:command clear */
        debugf("\033[2J");
        debugf("\033[1;1H");
        //
    }
}
//
/* Overview:
 *   Parse the next token from the string at s.
 *
 * Post-Condition:
 *   Set '*p1' to the beginning of the token and '*p2' to just past the token.
 *   Return:
 *     - 0 if the end of string is reached.
 *     - '<' for < (stdin redirection).
 *     - '>' for > (stdout redirection).
 *     - '|' for | (pipe).
 *     - 'w' for a word (command, argument, or file name).
 *
 *   The buffer is modified to turn the spaces after words into zero bytes
 * ('\0'), so that the returned token is a null-terminated string.
 */
int _gettoken(char* s, char** p1, char** p2) {
    *p1 = 0;
    *p2 = 0;
    if (s == 0) {
        return 0;
    }

    while (strchr(WHITESPACE, *s)) {
        *s++ = 0;
    }
    if (*s == 0) {
        return 0;
    }
    /* TODO:lab6-challenge */
    if (*s == '"') {
        *s = 0;
        *p1 = ++s;
        while (s != 0 && *s != '"') {
            s++;
        }
        if (s == 0) {
            debugf("\"don't match!!!\n");
            return 0;
        }
        *s = 0;
        *p2 = s;
        return 'w';
    }
    //
    if (strchr(SYMBOLS, *s)) {
        int t = *s;
        *p1 = s;
        *s++ = 0;
        *p2 = s;
        return t;
    }

    *p1 = s;
    while (*s && !strchr(WHITESPACE SYMBOLS, *s)) {
        s++;
    }
    *p2 = s;
    return 'w';
}

int gettoken(char* s, char** p1) {
    static int c, nc;
    static char *np1, *np2;

    if (s) {
        nc = _gettoken(s, &np1, &np2);
        return 0;
    }
    c = nc;
    *p1 = np1;
    nc = _gettoken(np2, &np1, &np2);
    return c;
}

#define MAXARGS 128
/* TODO:lab6-challenge */
int newDirCmd = 1;
//
int parsecmd(char** argv, int* rightpipe) {
    int forktemp = 0;
    int argc = 0;
    while (1) {
        char* t;
        int fd, r;
        int c = gettoken(0, &t);
        switch (c) {
            case 0:
                return argc;
            case 'w':
                if (argc >= MAXARGS) {
                    debugf("too many arguments\n");
                    exit();
                }
                /* TODO:lab6-challenge */
                argv[argc++] = t;
                // argv[argc++] = t;
                //
                break;
            case '<':
                if (gettoken(0, &t) != 'w') {
                    debugf("syntax error: < not followed by word\n");
                    exit();
                }
                // Open 't' for reading, dup it onto fd 0, and then close the
                // original fd.
                /* Exercise 6.5: Your code here. (1/3) */
                if ((fd = open(t, O_RDONLY)) < 0) {
                    r = fd;
                    user_panic("In runcmd: OPEN FAILED!");
                }
                dup(fd, 0);
                close(fd);
                //
                // user_panic("< redirection not implemented");

                break;
            case '>':
                if (gettoken(0, &t) != 'w') {
                    debugf("syntax error: > not followed by word\n");
                    exit();
                }
                // Open 't' for writing, dup it onto fd 1, and then close the
                // original fd.
                /* Exercise 6.5: Your code here. (2/3) */
                /* TODO:lab6-challenge */
                if ((fd = open(t, O_WRONLY | O_CREAT)) < 0) {
                    r = fd;
                    user_panic("In runcmd: OPEN FAILED!");
                }
                //
                dup(fd, 1);
                close(fd);
                //
                // user_panic("> redirection not implemented");

                break;
            case '|':;
                /*
                 * First, allocate a pipe.
                 * Then fork, set '*rightpipe' to the returned child envid or
                 * zero. The child runs the right side of the pipe:
                 * - dup the read end of the pipe onto 0
                 * - close the read end of the pipe
                 * - close the write end of the pipe
                 * - and 'return parsecmd(argv, rightpipe)' again, to parse the
                 * rest of the command line. The parent runs the left side of
                 * the pipe:
                 * - dup the write end of the pipe onto 1
                 * - close the write end of the pipe
                 * - close the read end of the pipe
                 * - and 'return argc', to execute the left of the pipeline.
                 */
                int p[2];
                /* Exercise 6.5: Your code here. (3/3) */
                pipe(p);
                if ((*rightpipe = fork()) == 0) {
                    dup(p[0], 0);
                    close(p[0]);
                    close(p[1]);
                    return parsecmd(argv, rightpipe);
                } else {
                    dup(p[1], 1);
                    close(p[1]);
                    close(p[0]);
                    return argc;
                }
                //
                // user_panic("| not implemented");

                break;
                // TODO:lab6-challenge
            case ';':
                forktemp = fork();
                if (forktemp) {
                    wait(forktemp);
                    return parsecmd(argv, rightpipe);
                } else {
                    return argc;
                }
                break;
            case '&':
                if ((r = fork()) == 0) {
                    return argc;
                } else {
                    return parsecmd(argv, rightpipe);
                }
                break;
                //
        }
    }
    return argc;
}

void runcmd(char* s) {
    gettoken(s, 0);

    char* argv[MAXARGS];
    int rightpipe = 0;
    int argc = parsecmd(argv, &rightpipe);
    if (argc == 0) {
        return;
    }
    /* TODO:lab6-challenge */
    if (strcmp(argv[0], "cd") == 0 || strcmp(argv[0], "cd.b") == 0 ||
        strcmp(argv[0], "clear") == 0) {
        do_incmd(argc, argv);
        return;
    }
    //
    argv[argc] = 0;
    int child = spawn(argv[0], argv);
    close_all();
    if (child >= 0) {
        /* TODO:lab6-challenge */
        wait(child);
    } else {
        debugf("spawn %s: %d\n", argv[0], child);
    }
    if (rightpipe) {
        wait(rightpipe);
    }
    exit();
}
/* TODO:lab6-challenge */
int offset;                             // 0:empty line, -1:last cmd, -2:...
int solveDirCmd(char* buf, int type) {  // type: 0 means up, 1 means down
    if (newDirCmd == 1) {
        offset = 0;
    }
    if (type == 0) {
        offset--;
    } else if (offset < 0) {
        offset++;
    }
    int x = 0;
    if (offset == 0) {
        while (buf[x] != '\0') {
            buf[x] = '\0';
            x++;
        }
        return -1;
    }
    int fdnum = open(".history", O_RDONLY);
    if (fdnum < 0) {
        debugf("open .history failed in sloveDir!\n");
        return 0;
    }
    struct Fd* fd = num2fd(fdnum);
    char* c;
    char* begin = fd2data(fd);
    char* end = begin + ((struct Filefd*)fd)->f_file.f_size;
    c = end - 1;

    while (((*c) == '\n' || (*c) == 0) && (c > begin)) {
        c--;
    }

    if (c == begin) {  // no history cmd
        buf[0] = '\0';
        return 0;
    }

    c++;  // last \n or \0
    int i;
    for (i = 0; i > offset; i--) {
        while ((*c) != '\n' && (*c) != '\0') {
            c--;
            if (c <= begin) {
                break;
            }
        }
        c--;
        if (c <= begin) {
            break;
        }
    }
    offset = i;  // avoid offset too bigger than real cmd num
    if (c > begin) {
        while (c > begin && (*c) != '\n') {
            c--;
        }
        if ((*c) == '\n') {
            c++;
        }
    } else {
        c = begin;
    }
    int now = 0;
    while (buf[now] != '\0') {
        buf[now] = '\0';
        now++;
    }
    now = 0;
    while ((*c) != '\n' && (*c) != '\0' && (*c) < end) {
        buf[now] = *c;
        now++;
        c++;
    }
    return now;
}
//
#define MOVELEFT(y) printf("\033[%dD", (y))
#define MOVERIGHT(y) printf("\033[%dC", (y))
void readline(char* buf, u_int n) {
    /* TODO:lab6-challenge */
    int r;
    int off = 0;
    int len = 0;
    char op;
    while (off < n) {
        if ((r = read(0, &op, 1)) != 1) {
            if (r < 0) {
                debugf("read error: %d\n", r);
            }
            exit();
        }
        if (op == '\b' || op == 0x7f) {
            /* TODO:lab6-challenge */
            if (off > 0) {
                if (off == len) {
                    buf[--off] = 0;
                    printf("\033[D \033[D");
                } else {
                    for (int j = off - 1; j < len - 1; j++) {
                        buf[j] = buf[j + 1];
                    }
                    buf[len - 1] = 0;
                    MOVELEFT(off--);
                    printf("%s ", buf);
                    MOVELEFT(len - off);
                }
                len -= 1;
            }
            //
        } else if (op == '\r' || op == '\n') {
            buf[len] = 0;
            return;
        }
        /* TODO:lab6-challenge */
        else if (op == 27) {
            char tmp;
            read(0, &tmp, 1);
            char tmp2;
            read(0, &tmp2, 1);
            if (tmp == 91 && tmp2 == 65) {
                debugf("\x1b[B");  // down to cmd line
                int j;
                for (j = 0; j < off; j++) {
                    debugf("\x1b[D");  // left to line head
                }
                debugf("\x1b[K");  // clean line
                off = solveDirCmd(buf, 0);
                len = strlen(buf);
                debugf("%s", buf);
            } else if (tmp == 91 && tmp2 == 66) {
                int j;
                for (j = 0; j < off; j++) {
                    debugf("\x1b[D");
                }
                debugf("\x1b[K");
                off = solveDirCmd(buf, 1);
                len = strlen(buf);
                debugf("%s", buf);
            }
            /* TODO:lab6-challenge */
            else if (tmp == 91 && tmp2 == 67) {
                if (off < len) {
                    off++;
                } else {
                    MOVELEFT(1);
                }
            } else if (tmp == 91 && tmp2 == 68) {
                if (off > 0) {
                    off--;
                } else {
                    MOVERIGHT(1);
                }
            }
            //
            newDirCmd = 0;
        } else {
            newDirCmd = 1;
            if (off == len) {
                buf[off++] = op;
            } else {  // i < len
                for (int j = len; j > off; j--) {
                    buf[j] = buf[j - 1];
                }
                buf[off] = op;
                buf[len + 1] = 0;
                MOVELEFT(++off);
                printf("%s", buf);
                MOVELEFT(len - off + 1);
            }
            len += 1;
        }
    }
    debugf("line too long\n");
    while ((r = read(0, buf, 1)) == 1 && buf[0] != '\r' && buf[0] != '\n') {
        ;
    }
    buf[0] = 0;
}

char buf[1024];

void usage(void) {
    debugf("usage: sh [-dix] [command-file]\n");
    exit();
}
/* TODO:lab6-challenge */
void save_cmd(char* cmd) {
    int r = open(".history", O_CREAT | O_WRONLY | O_APPEND);
    if (r < 0) {
        debugf("open .history failed! in save");
        return r;
    }
    write(r, cmd, strlen(cmd));
    write(r, "\n", 1);
    return 0;
}
void clarify_cmd(char* cmd) {
    int len = strlen(cmd);
    for (int i = 0; i < len; i++) {
        if ((cmd[i] == 'l' && cmd[i + 1] == 's' &&
             (i + 3 >= len || cmd[i + 2] != '.' && cmd[i + 3] != 'b')) ||
            (cmd[i] == 'c' && cmd[i + 1] == 'd' &&
             (i + 3 >= len || cmd[i + 2] != '.' && cmd[i + 3] != 'b'))) {
            len += 2;
            for (int j = len - 1; j > i + 1; j--) {
                cmd[j] = cmd[j - 2];
            }
            cmd[i + 2] = '.';
            cmd[i + 3] = 'b';
        }
    }
    for (int i = 0; i < len; i++) {
        if ((cmd[i] == 'c' && cmd[i + 1] == 'a' && cmd[i + 2] == 't' &&
             (i + 4 >= len || cmd[i + 3] != '.' && cmd[i + 4] != 'b')) ||
            (cmd[i] == 'p' && cmd[i + 1] == 'w' && cmd[i + 2] == 'd' &&
             (i + 4 >= len || cmd[i + 3] != '.' && cmd[i + 4] != 'b'))) {
            len += 2;
            for (int j = len - 1; j > i + 2; j--) {
                cmd[j] = cmd[j - 2];
            }
            cmd[i + 3] = '.';
            cmd[i + 4] = 'b';
        }
    }
    for (int i = 0; i < len; i++) {
        if ((cmd[i] == 'e' && cmd[i + 1] == 'c' && cmd[i + 2] == 'h' &&
             cmd[i + 3] == 'o' &&
             (i + 5 >= len || cmd[i + 4] != '.' && cmd[i + 5] != 'b')) ||
            (cmd[i] == 'h' && cmd[i + 1] == 'a' && cmd[i + 2] == 'l' &&
             cmd[i + 3] == 't' &&
             (i + 5 >= len || cmd[i + 4] != '.' && cmd[i + 5] != 'b')) ||
            (cmd[i] == 't' && cmd[i + 1] == 'r' && cmd[i + 2] == 'e' &&
             cmd[i + 3] == 'e' &&
             (i + 5 >= len || cmd[i + 4] != '.' && cmd[i + 5] != 'b'))) {
            len += 2;
            for (int j = len - 1; j > i + 3; j--) {
                cmd[j] = cmd[j - 2];
            }
            cmd[i + 4] = '.';
            cmd[i + 5] = 'b';
        }
    }
    for (int i = 0; i < len; i++) {
        if ((cmd[i] == 'm' && cmd[i + 1] == 'k' && cmd[i + 2] == 'd' &&
             cmd[i + 3] == 'i' && cmd[i + 4] == 'r' &&
             (i + 6 >= len || cmd[i + 5] != '.' && cmd[i + 6] != 'b')) ||
            (cmd[i] == 't' && cmd[i + 1] == 'o' && cmd[i + 2] == 'u' &&
             cmd[i + 3] == 'c' && cmd[i + 4] == 'h' &&
             (i + 6 >= len || cmd[i + 5] != '.' && cmd[i + 6] != 'b'))) {
            len += 2;
            for (int j = len - 1; j > i + 4; j--) {
                cmd[j] = cmd[j - 2];
            }
            cmd[i + 5] = '.';
            cmd[i + 6] = 'b';
        }
    }
    for (int i = 0; i < len; i++) {
        if ((cmd[i] == 'h' && cmd[i + 1] == 'i' && cmd[i + 2] == 's' &&
             cmd[i + 3] == 't' && cmd[i + 4] == 'o' && cmd[i + 5] == 'r' &&
             cmd[i + 6] == 'y' &&
             (i + 8 >= len || cmd[i + 7] != '.' && cmd[i + 8] != 'b'))) {
            len += 2;
            for (int j = len - 1; j > i + 6; j--) {
                cmd[j] = cmd[j - 2];
            }
            cmd[i + 7] = '.';
            cmd[i + 8] = 'b';
        }
    }
}
//
int main(int argc, char** argv) {
    int r;
    int interactive = iscons(0);
    int echocmds = 0;
    debugf("\033[32m\n::::::::::::::::::::::::::::::::::::::::::::::\n\033[m");
    debugf("\033[32m::                                          ::\n\033[m");
    debugf("\033[32m::                                          ::\n\033[m");
    debugf("\033[32m::                                          ::\n\033[m");
    debugf("\033[32m::          Charles Shell 2023              ::\n\033[m");
    debugf("\033[32m::                21373191                  ::\n\033[m");
    debugf("\033[32m::                                          ::\n\033[m");
    debugf("\033[32m::                                          ::\n\033[m");
    debugf("\033[32m::                    charles2530@github.io ::\n\033[m");
    debugf("\033[32m::::::::::::::::::::::::::::::::::::::::::::::\n\033[m");
    ARGBEGIN {
        case 'i':
            interactive = 1;
            break;
        case 'x':
            echocmds = 1;
            break;
        default:
            usage();
    }
    ARGEND

    if (argc > 1) {
        usage();
    }
    if (argc == 1) {
        close(0);
        if ((r = open(argv[1], O_RDONLY)) < 0) {
            user_panic("open %s: %d", argv[1], r);
        }
        user_assert(r == 0);
    }
    /* TODO:lab6-challenge */
    if ((r = chdir("/")) < 0) {
        printf("created root path failed: %d\n", r);
    }
    //
    for (;;) {
        if (interactive) {
            printf("\033[35m\nCharles\033[m\033[36m $ \033[m");
        }
        readline(buf, sizeof buf);
        /* TODO:lab6-challenge */
        save_cmd(buf);
        // clarify_cmd(buf);
        //
        if (buf[0] == '#') {
            continue;
        }
        if (echocmds) {
            printf("# %s\n", buf);
        }
        /* TODO:lab6-challenge */
        int len = strlen(buf);
        if (len >= 2 && buf[0] == 'c' && buf[1] == 'd' ||
            len >= 5 && buf[0] == 'c' && buf[1] == 'l' && buf[2] == 'e' &&
                buf[3] == 'a' && buf[4] == 'r') {
            // internal command
            runcmd(buf);
        } else {
            if ((r = fork()) < 0) {
                user_panic("fork: %d", r);
            }
            if (r == 0) {
                runcmd(buf);
                exit();
            } else {
                wait(r);
            }
        }
        //
    }
    return 0;
}
