#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <linux/if.h>

int get_iface_list(struct ifconf *ifconf)
{
   int sock, rval;

   sock = socket(AF_INET,SOCK_STREAM,0);
   if(sock < 0)
   {
     perror("socket");
     return (-1);
   }

   if((rval = ioctl(sock, SIOCGIFCONF , (char*) ifconf  )) < 0 )
     perror("ioctl(SIOGIFCONF)");

   close(sock);

   return rval;
}

int main()
{
   static struct ifreq ifreqs[20];
   struct ifconf ifconf;
   int  nifaces, i, rc;

   memset(&ifconf,0,sizeof(ifconf));
   ifconf.ifc_buf = (char*) (ifreqs);
   ifconf.ifc_len = sizeof(ifreqs);

   if(get_iface_list(&ifconf) < 0) exit(-1);

   nifaces =  ifconf.ifc_len/sizeof(struct ifreq);

   printf("Interfaces (count = %d):\n", nifaces);
   for(i = 0; i < nifaces; i++)
   {
//     rc = ioctl(sockfd, SIOCGIFADDR,  (void *) ifreqs[i]);
//                if (rc < 0) {
//                        sprintf(stderr, "get_myipaddr: ioctl(SIOCGIFADDR):",);
//                        return(-1);
//                }

     printf("\t%-10s\n", ifreqs[i].ifr_name);
   }
}
