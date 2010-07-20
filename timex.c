#include <stdio.h>
#include <time.h>
#include <stdlib.h>
#include <string.h>
#include "queue.h"

int main(int argc, char **argv) {
    queue_t *q = NULL;
    long allocations = 1000000;
    long count = 0, i = 0;
    queue_elem_t *x = NULL;


    q = queue_init();

    printf("CLOCKS_PER_SEC is %ld\n", CLOCKS_PER_SEC);

    clock_t uptime = clock();
    printf("Uptime is %ld\n", uptime);

    uptime = clock();
    printf("Uptime after uptime call is %ld\n", uptime);

    for( i = 0; i < allocations; ++i) {
        queue_push(q, (ino_t) count++);
    }

    uptime = clock();
    printf("Uptime after %ld allocations is %ld\n", allocations, uptime);

    for( x = q->queue; x != NULL; x = x->next) {
        ;
    }

    uptime = clock();
    printf("Uptime after %ld lookups is %ld\n", allocations, uptime);
}
