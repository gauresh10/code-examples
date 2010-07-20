#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#define HWLEN 20
#define ADDRLEN 16
#define TO_CP 8

typedef struct {
    int h1;
    long h2;
    char h3;
    unsigned char hwaddr[HWLEN];
} iface_t;

typedef struct {
    int h1;
    int h2;
    char h3;
    char h4;
    char h5;
    unsigned char cwaddr[ADDRLEN];
} message_t;


int main(int argc, char *argv[]) {

  message_t *target = (message_t *) malloc(sizeof(message_t));
  iface_t   *source = (iface_t *) malloc(sizeof(iface_t));

  strncpy(&source->hwaddr, "01020304050607080911121314151617181920", HWLEN);
  printf("Source addr is %s(%p) %p(%d) %p(%d)\n", &source->hwaddr, &source->hwaddr, &((source->hwaddr)[12]), sizeof((source->hwaddr)[12]), (&(source->hwaddr)) + 12, sizeof(source->hwaddr));

  printf("0x60205d - 0x602051 = %d\n0x602141 - 0x602051 = %d\n", 0x60205d - 0x602051, 0x602141 - 0x602051);
  memcpy(&target->cwaddr, &source->hwaddr[0] + 12, TO_CP);
  printf("Target addr is %s addr %p begins with %c\n", &target->cwaddr, target->cwaddr, &target->cwaddr);

  memcpy(&target->cwaddr, &source->hwaddr[12], TO_CP);
  printf("Target addr is %s addr %p begins with %c\n", &target->cwaddr, target->cwaddr, &target->cwaddr);
  return 0;
}

