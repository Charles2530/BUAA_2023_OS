/* TODO:lab6-challenge */
#include <lib.h>
int main(int argc, char** argv) {
    if (argc != 1) {
        debugf("usage: history\n");
    }
    int fdnum = open(".history", O_RDONLY);
    if (fdnum < 0) {
        debugf("open .history failed\n");
        return;
    }
    char buf;
    int r;
    int cnt = 0;
    int newline = 1;
    while ((r = read(fdnum, &buf, 1)) != 0) {
        if (newline) {
            debugf("\033[32mHistory \033[m");
            debugf("\033[34m%d \033[m", cnt);
            debugf("\033[32m: \033[m");
            debugf("%c", buf);
            cnt++;
            newline = 0;
        } else {
            debugf("%c", buf);
        }
        if (buf == '\n') {
            newline = 1;
        }
    }
}