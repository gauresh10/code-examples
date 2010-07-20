#include <stdio.h>
#include <stdlib.h>

typedef struct {
    void *x;
    
} my_fake_t;

void *my_realloc(void *p)
{

    printf("Before %p\n", p);
    p = realloc(p, sizeof(int) * 10);
    printf("After %p\n", p);
    return (p);
}


int main(int argc, char *argv[])
{
    void *p;
    p = malloc(sizeof(int) * 20);
    printf("Malloc %p\n", p);
    p = my_realloc(p);
    printf("Realloc %p\n", p);

    free(p);

    printf("%ld %ld %p\n", 0xffffffff81357000, 0x7fe681357000, (void *) -1);
    return 0;
}

