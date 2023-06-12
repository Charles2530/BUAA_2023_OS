/* TODO:lab6-challenge */
#include <lib.h>

void usage(void) {
    printf("usage: pwd\n");
    exit();
}

int main(int argc, char** argv) {
    int r;
    char buf[128];

    if (argc != 1) {
        usage();
    } else {
        if ((r = getcwd(buf)) < 0) {
            printf("get path failed: %d\n", r);
            exit();
        }
        printf("\033[34m%s\n\033[m", buf);
    }

    return 0;
}
