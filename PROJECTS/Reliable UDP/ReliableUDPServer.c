#include <stdio.h>
#include <fcntl.h>
#include <signal.h>	
#include <string.h>
#include <unistd.h>                                  /*Header files*/
#include <stdlib.h>
#include <stdbool.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/socket.h> 
#include <netinet/in.h>


#define maxbuf 1024*1024                           /*Defining the maximum buffer size*/
#define maxBuffer1 500*500

FILE *checkpoint;                                  /*File pointer*/
                                        
struct sockaddr_in clientAddr;
struct itimerval it_val;
struct timeval  tv1, tv2;

socklen_t clientLength;

char buf[maxbuf];
char *fileName;

int serverseqno = 2700;                                    /*seq no: passed in the sender's header*/
int expectedackno,sock,packetcount,dupcount;
long a = 0L;                                               /*to track the position inside the file*/
int adw;
 
void DoStuff(void)                              /*Enters this loop after the timeout*/

{

  printf("Timer went off.\n");
  it_val.it_value.tv_sec = 0;
  it_val.it_value.tv_usec = 0;                  /*the timer is reset to zero*/
  
  if(serverseqno > expectedackno)             /*if expected ackno is lesser than seq no, then packet is retransmitted*/
  {
	 int d = expectedackno/300;
	 a = a + d;
	 fseek(checkpoint, a, SEEK_CUR);  
  }

}


double timeout(double o)                                      /*Timeout interval is calculated based on the below formulas*/
{
	double samplertt = o;
	/* printf("Sample rtt: %f \n", samplertt); */
	
	double estimatedrtt = (double)( (double) (0.875)* (double) (estimatedrtt)) + (double)((double) (0.125) *(double)(samplertt));
	/* printf("estimatedrtt: %f \n", estimatedrtt); */
	double dif = (double) ((double) (samplertt) - (double) (estimatedrtt));
	/* printf("Absolute values: %f \n", dif); */
	double devrtt = (0.25 * devrtt) + (0.75 * dif);
	/* printf ("Devrtt: %f \n", devrtt); */
	
	double timeoutinterval = estimatedrtt + (4 * devrtt) ;
	/* printf("Time out interval: %f", timeoutinterval); */
	return timeoutinterval;
}


int CheckFileDir(int sock)                       /* Opens the file to read */
{
    
	char serverDirectory[50] = "/u/vensamba/bin";           /* Root Directory where the file is stored */  
	strcat(serverDirectory, fileName);
	if ( (checkpoint = fopen(serverDirectory, "r")) == NULL )        /*  Checks whether the file exists */       
		{
		printf("File not found!\n");	
	    }
		
		 Forward(sock);                                      /* Create and send HTTP response */
		
	return 0;
}



int Receive(char * buf)            /* Gets the file name */
{
	
    char      *rem;
    int        fileNameLength;

	buf += 5; 

	rem = strchr(buf, ' ');
	fileNameLength = rem - buf;
	
	if ( fileNameLength == 1 )                                        /*if file length is 1, then file not found*/
	{
		printf("File not found \n"); 
	}
	
	fileName = calloc(fileNameLength + 1, sizeof(char));
	strncpy(fileName, buf, fileNameLength);
	
	
	char *ret1;
		 
			ret1 = strstr(buf,"Advertized window size: ");                            /*gets the full line*/
		 
		ret1 +=24;
		
		adw = atoi(ret1); 
printf(" \n\n Advertized window size: %d \n\n\n" , adw);	
		
	
	CheckFileDir(sock);                                        /* Get requested file from the server */
	
    return 0;
}

int SenderHeaders(int sendResponse)              /* Sender Headers */
{
	bzero(buf,maxbuf);
	sprintf(buf,"Sequence no: %d \n",serverseqno);                    /*Seq no: associated with every packet*/
	
    return 0;
}





int Forward(int sock)                    /*Send the response to the client's request*/
{
    char filedata[50000];
	char buffer1[20000];char buffer2[50];char buffer3[10000];char buf[5048576];
	char data;
	int fileLength,i=0;
	int window =adw;                                       /*assume the eadvertized window is 10*/
    int w=0;
    int v=0;

    
	SenderHeaders(sock);              /*  Send headers */
	int noofpackets = window;                                 /*Sending that many no. of packets bases on the window size*/
	
		fseek(checkpoint, 0L, SEEK_END);                      
		fileLength = ftell(checkpoint);
		int count = fileLength/300;                  /*Calculates the number of packets in a file, 300 bytes in each packet*/
		int j=0; 
		fseek(checkpoint, 0L, SEEK_SET);
		
	 /*fseek Sets the starting and ending points to read*/
	 
		bzero(buf,strlen(buf));
		

	printf("no of packets: %d \n", count);           
	int total = count/10;
	printf("Total: %d", total);                         
	
     
	for(i=0;i<total + 1;i++)                              /*  Read the packets */    
	{
		w = w+1;
		for(j=1;j<=adw;j++)                                   /* Service 300 bytes at a time */
		{ 
			
			++packetcount;
			printf("packetcount: %d \n",packetcount);
			if(packetcount==count +1) 
			{
				strcpy(buf, "Transmitting the last byte: \nSEEKEND\n");
	sendto(sock,buf,strlen(buf),0,(struct sockaddr *)&clientAddr,clientLength);

	printf("All the packets have been read\n");
	fclose(checkpoint);
	close(sock); 
	/* exit(1); */
			};
			
		
		if (fileLength- (j*300)<300) fread(filedata, 1, fileLength- (j*300), checkpoint);    
		else fread(filedata, 1, 300, checkpoint);                                   /*reads 300 packets at a time*/
		 strcat(buffer3,filedata); 
		
		sprintf(buffer1,"\n New server Seq no : /%d \r\n", serverseqno);
		serverseqno =serverseqno +300;
		
			strcat(buffer1,buffer3);
			strcat(buf,buffer1);
		bzero(buffer1,strlen(buffer1));
		bzero(buffer3,strlen(buffer3));
		
		 
		sendto(sock,buf,strlen(buf),0,(struct sockaddr *)&clientAddr,clientLength);
		bzero(buf,strlen(buf));
			
		}
		

  
		a = a + 3000L;
		gettimeofday(&tv1, NULL);                 /*to calculate samplrtt, calculate starting time after sending packet*/
		recvfrom(sock,buffer2,maxbuf,0,(struct sockaddr *)&clientAddr,&clientLength);

		gettimeofday(&tv2, NULL);                 /*to calculate samplrtt, calculate ending time after receiving the ack*/
		double m = (double) (tv2.tv_usec - tv1.tv_usec) / 1000000;
		double n = (double) (tv2.tv_sec - tv1.tv_sec);
		double o = m + n;                          /* This gives the sample rtt value */
		double t = timeout(o);                     /*Sample rtt is used to calculate the time out interval*/
	    printf ("Timeout = %f seconds\n",t);
		printf("Acknowledgement received : %s \n",buffer2);
		
		
			/*start the timer and check if packet is packet was sent and ack was received successfully*/
		
		if (signal(SIGALRM, (void (*)(int)) DoStuff) == SIG_ERR)                       
  {
    perror("Could not catch the SIGALRM");
    exit(1);
  }
    int tt = t * 1000;
    it_val.it_value.tv_sec =     tt/1000;
    it_val.it_value.tv_usec =    (tt*1000) % 1000000;	
    it_val.it_interval = it_val.it_value;
	
	
		expectedackno = atoi(buffer2);
		
		if(serverseqno > expectedackno)           /*if expectedackno < serverseqno, then increase the dup ack by 1*/
		{	
	      dupcount ++;
		  if(dupcount == 3)                       /* if the dup ack count reaches 3, then resent those packets */
		   {
			 int d = expectedackno/300;
		     a = a + d;
		     fseek(checkpoint, a, SEEK_CUR);
		     dupcount = 0;
		   }
		}
		
		/* if(strstr(buffer2,"Packet lost")!=NULL)
		{
			i=i-1;
			a = a - 3000L;
			fseek(checkpoint, a, SEEK_CUR);
		} */
        
		bzero(buffer2,strlen(buffer2));
			
		if(w==total + 1)
			break;	
	}
		
	strcpy(buf, "Transmitting the last byte: \nSEEKEND\n");    /*  Indicates the end of the file */
	sendto(sock,buf,strlen(buf),0,(struct sockaddr *)&clientAddr,clientLength);

	printf("Transmitted the last byte:\n");
	fclose(checkpoint);
    return 0;


}

int main(int argc, char **argv) 
{
	
  int output;	
  struct sockaddr_in serverAddr;
  
  
  if (argc < 2)                                      /*Checks if the port number was given at the command line*/
  {
    printf(" Please provide the port number \n");
    exit(1); 
  }


  sock = socket(AF_INET, SOCK_DGRAM, 0);           /* Creates the socket */
  
  if (sock < 0) 
  {
    printf("Socket could not be opened \n");               /* If sock < 0 , encountered an issue with the socket */
  }
  
  bzero((char *) &serverAddr, sizeof(serverAddr));
  
  
 serverAddr.sin_family = AF_INET;
  

  serverAddr.sin_port = htons(atoi(argv[1]));                   
  
  /* ATOI: Converts the port number from a string to an integer data type, HTONS: converts it to a network byte order */
 
 serverAddr.sin_addr.s_addr = INADDR_ANY;
 

  if (bind(sock, (struct sockaddr *) &serverAddr, sizeof(serverAddr)) < 0)    /* Binds the socket */
  {
    printf("Binding error \n");
	exit(1);
  }
  
  clientLength = sizeof(clientAddr);
 
  bzero(buf,maxbuf);                                     /* Bzero will empty the buffer */
 
  output = recvfrom(sock,buf,maxbuf,0,(struct sockaddr *)&clientAddr,&clientLength);
 
  if (output < 0) 
	  printf("Issues receiving the data from the client");
 
  int i;

  Receive(buf);                             /* Receives the client headers */ 
 
  close(sock);                              /* Closes the socket */	
  return 0;
}


