/*UDP: Client*/

#include <stdio.h>                                                      
#include <string.h>
#include <sys/types.h>
#include <stdlib.h>
#include <unistd.h>
#include <netdb.h>
#include <errno.h> 
#include <netinet/in.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <arpa/inet.h>
#include <string.h>
#include <sys/time.h>

void error(const char *msg)                                                          /*Dislays the error message*/
{
    perror(msg);
    exit(0);
}

  int main(int argc, char **argv)
{
   int sock, n;
   unsigned int length;
   struct sockaddr_in server, from;
   struct hostent *hp;
   char buffer[256];
   
   if (argc < 3) { printf("Usage: server port\n");
                    exit(1);
   }
   sock= socket(AF_INET, SOCK_DGRAM, 0);
   if (sock < 0) error("socket");

   server.sin_family = AF_INET;
   hp = gethostbyname(argv[1]);
   if (hp==0) error("Unknown host");

   bcopy((char *)hp->h_addr, 
        (char *)&server.sin_addr,
         hp->h_length);
   server.sin_port = htons(atoi(argv[2]));
   length=sizeof(struct sockaddr_in);
 
   
    sprintf(buffer, "GET /%s HTTP/1.1\r\n", argv[4]);
    sendto(sock, buffer, strlen(buffer),0,(const struct sockaddr *)&server,length);
	bzero(buffer,500);                                                                      /*clear the buffer*/
	sendto(sock, "Connection: close\r\n", 24,0,(const struct sockaddr *)&server,length);
	sprintf(buffer, "Host: silo.soic.indiana.edu:%d\r\n", argv[2]);
	sendto(sock, buffer, strlen(buffer),0,(const struct sockaddr *)&server,length);
	sendto(sock, "User-Agent: Chrome/15.0\r\n", 27,0,(const struct sockaddr *)&server,length);
	sendto(sock, "Accept-Language: en-US\r\n", 24,0,(const struct sockaddr *)&server,length);      /*Writing the header requests to the server*/
    sendto(sock, "\r\n", 2,0,(const struct sockaddr *)&server,length);
	sendto(sock, "\r\n", 2,0,(const struct sockaddr *)&server,length);
	
   if (n < 0) error("Sendto");
   recvfrom(sock,buffer,1000,0,(struct sockaddr *)&from, &length);
   if (n < 0) error("recvfrom");
   printf(buffer);
   close(sock);
   return 0;
}