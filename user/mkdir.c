/* TODO:lab6-challenge */
#include <lib.h>

int main(int argc, char** argv) {
    if (argc == 2) {
        if (create_dir(argv[1]) < 0) {
            printf("create dir failed!\n");
        } else {
            printf("\033[35mSuccess mkdir\033[m\n");
        }
    } else {
        printf("usage: mkdir [dirname]\n");
    }
    return 0;
}
