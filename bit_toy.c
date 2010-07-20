#include <stdio.h>
#include <stdlib.h>

int main(int argc, char **argv){
    long x;

    if (argc > 1) {
        x = atoi(argv[1]);
        printf("Given: %#x\n", x);
    } else {
        printf("No argument provided\n");
    }
    return 0;
}