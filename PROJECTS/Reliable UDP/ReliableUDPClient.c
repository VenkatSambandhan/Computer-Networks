#include <stdio.h>
#include <fcntl.h>
#include <netdb.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>                                    /*Header files*/
#include <netinet/in.h>
#include <sys/socket.h>

#define bufsize 1024*1024                                /* Maximum buffer size */

struct sockaddr_in serverAddr,clientAddr;

char buffer[bufsize]; char buffer1[30]; char buffer2[30];

int clientackno=3000, expectedseqno = 3000, m = 3000;
int length, total=0;
int array1[10];



void ClientHeaders(char *p, char *a, char * w, int sendSock)                /* Sends the client headers to the server */
{
	char *file;
    char *port;
	char *adw;
	port = p;
	file = a;
	adw = w;
	
	bzero(buffer,bufsize);
    sprintf(buffer, "File /%s \r\n", file);
	strcat(buffer, port);
	strcat(buffer,"\r\n");
    sprintf(buffer1, "Starting Clientackno: %d \r\n", clientackno); 
	strcat(buffer, buffer1);
	int adw1 = atoi(adw);
	sprintf(buffer2, "Advertized window size: %d \r\n", adw1); 
	
	printf("Advertized window: %d",adw1 );
	strcat(buffer, buffer2);
    sendto(sendSock,buffer,strlen(buffer),0,(const struct sockaddr *)&serverAddr,length);
}

int main(int argc, char **argv)
{
    int sockfd;
    struct timeval start, end;
	struct hostent *server;
	
/* Checks if the arguments are inputted correctly.. */

    if(argc < 2)
	{
		printf("Enter the correct server name\n");
		exit(1);
	}
	
	if (argc < 3) 
	{
       printf("Enter the correct port number\n", argv[0]);
       exit(1);
    }
	
    if (argc < 4) 
	{
       printf("Enter the filename\n");
       exit(1);
	   
	   if(argc < 5)
	   {
		 printf("Enter the advertized window size\n");  
         exit(1);		 
	   }
    }
  
    sockfd = socket(AF_INET, SOCK_DGRAM, 0);                         /* Creates the socket */
    
	if (sockfd < 0) 
        printf("Problem opening socket \n");
	
    server = gethostbyname(argv[1]);
    if (server == NULL) {
        printf("Could not find the host \n");
        exit(1);
    }
	
    bzero((char *) &serverAddr, sizeof(serverAddr));
    serverAddr.sin_family = AF_INET;
    bcopy((char *)server->h_addr, (char *)&serverAddr.sin_addr.s_addr, server->h_length);
	
    serverAddr.sin_port = htons(atoi(argv[2]));            /* Change the port number from string to an integer*/
	
    length=sizeof(struct sockaddr_in);
	
	ClientHeaders(argv[2], argv[3], argv[4],sockfd);

	int i,k; char buffer1[30];
	
	while(1)                                                      /* Read the HTTP response */
	{
		
		bzero(buffer,bufsize);
		
		for(i=0;i<=9;i++)
		{
		
		k = recvfrom(sockfd,buffer,bufsize,0,(struct sockaddr *)&clientAddr, &length);
		if (k < 0) printf("Recvfrom function encountered an error");
		
	    	char *ret1;
		 
			ret1 = strstr(buffer,"New server Seq no ");                            /*gets the full line*/
		 
		ret1 +=21;
		
		int a = atoi(ret1);                                   /* Convert from string to integer */
		
		int y = a + 300;
	
		if(clientackno == y)                                   /* Checks if it is the expected sequence no */
		{	
	       array1[i]=1;                                      
		   printf("buffer %s",buffer); 
	       printf(" \n Acknowledged.. \n");
	   
	   /*if its the correct the seq no, then it is printed, and the array value is set to 1, for checking out of order packets.*/
		
		}
		else
		{
			array1[i]=0;
			printf(" \n\n Not Acknowledged.. \n\n");
		/*if its the wrong the seq no, the array value is set to 0, for retransmission at the sender side.*/	
			
		}
		clientackno = clientackno +300;                             /*the clientack no is updated every time*/
		bzero(buffer1,strlen(buffer1));                             /* clearing the buffer */
		
		bzero(buffer,bufsize);                                      /* clearing the buffer */
		
		 /* If packet lost, then the expected sequence no is sent, so that it can retransmitted after 3 duplicat acks*/
		 
		 if(a!= expectedseqno)                                       
		{
			sprintf(buffer1, "%d", expectedseqno); 
			sendto(sockfd,buffer1,strlen(buffer1),0,(const struct sockaddr *)&serverAddr,length);
		}
		
		/*If transmitted successfully, then just increment it to check for the next sequence*/
		
		else 
		{
			expectedseqno = expectedseqno + 300;
		}
		
		}
		
		/* Prints the array values */
		for(i=0;i<10;i++)
		{
			printf(" Array values: %d \t", array1[i]);
		
		/* If the value is 0, then calculating the sequence number based on the 0th position, and sending the seq no, so that all those packets can be retransmitted */
		
		if(array1[i]==0)
		 {
			int q=i*300; 	
			int n = m + q;
			  printf(" \n Value of q: %d \n", n);
			  sprintf(buffer, "%d", n); 
		
		 sendto(sockfd,buffer,strlen(buffer),0,(const struct sockaddr *)&serverAddr,length);
		
		 break;
		 }
	
		}
		
		 if(clientackno % 3000 == 0)               /*If all 10 packets are received successfully*/
		{
			
		sprintf(buffer, "%d", clientackno); 
		
		 sendto(sockfd,buffer,strlen(buffer),0,(const struct sockaddr *)&serverAddr,length);   /*sends the buffer to the server*/
		
		}
		
		/* else
		{
			
			char resend[]="Packet lost";
			strcat(buffer,resend); 
		 printf("buffer %s",buffer);  
		 sendto(sockfd,buffer,strlen(buffer),0,(const struct sockaddr *)&serverAddr,length);
		 clientackno1 = clientackno1 +3000; 
			} */
				
		if(strstr(buffer,"SEEKEND")!=NULL) break;            /*End of the HTTP response */         
				
			
	m = m +3000;
	}

       close(sockfd);                                    /*close the file descriptor*/
    return 0;
}
