/* TODO:lab6-challenge */
#include <lib.h>

int main(int argc, char** argv) {
    if (argc == 2) {
        if (create_file(argv[1]) < 0) {
            printf("create file failed!\n");
        } else {
            printf("\033[35mSuccess touch\033[m\n");
        }
    } else {
        printf("usage: touch [filename]\n");
    }
    return 0;
}
