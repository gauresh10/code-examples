#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <time.h>
#include <string.h>
#include <stdio.h>


#define MULTICAST_PORT 3535
#define MULTICAST_GROUP "225.0.0.35"
#define MSGBUFSIZE 4256

int main(int argc, char *argv[])
{
        struct sockaddr_in addr;
        int                fd, nbytes, addrlen;
        struct             ip_mreq mreq;
        char               msgbuf[MSGBUFSIZE];

        u_int yes=1;            /*** MODIFICATION TO ORIGINAL */

    if ((fd=socket(AF_INET,SOCK_DGRAM,0)) < 0) {
        perror("socket");
        return -1;
    }

    if (setsockopt(fd,SOL_SOCKET,SO_REUSEADDR,&yes,sizeof(yes)) < 0) {
        perror("Reusing ADDR failed");
        return -1;
    }

    memset(&addr,0,sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = htonl(INADDR_ANY); /* N.B.: differs from sender */
    addr.sin_port = htons( argc == 3 ? (int) atoi(argv(2)) : MULTICAST_PORT);

    if (bind(fd, (struct sockaddr *) &addr, sizeof(addr)) < 0) {
        perror("bind");
        return -1;
    }

    mreq.imr_multiaddr.s_addr = inet_addr(argc == 3 ? argv(1) : MULTICAST_GROUP);
    mreq.imr_interface.s_addr = htonl(INADDR_ANY);
    if (setsockopt(fd, IPPROTO_IP, IP_ADD_MEMBERSHIP, &mreq, sizeof(mreq)) < 0) {
        perror("setsockopt");
        return -1;
    }

    while (1) {
        addrlen=sizeof(addr);
        if ((nbytes=recvfrom(fd, msgbuf, MSGBUFSIZE, 0,
                   (struct sockaddr *) &addr, &addrlen)) < 0) {
            perror("recvfrom");
            return -1;
        }
        printf("got %d bytes : %s\n", nbytes, msgbuf);
    }
    return 0;
}
