/*UDP: Server*/

#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <errno.h>
#include <sys/types.h>
#include <fcntl.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <string.h>
#include <netdb.h>
#include <sys/time.h>

/*Global variables*/
                                         
int fl = 1, status = 200, method = 0;            /*f1 repesents the first header line*/ /*0 means method is get*/
char CS[] = "0";                                                 /*0 represents connection status as keep alive*/
char *file;
socklen_t fromlen;
struct sockaddr_in from; 
struct sockaddr_in server;
struct sockaddr_in from;    
int length = sizeof(server);
                                                        

void error(const char *fault)                                            /*This function displays the error message*/
{
    perror(fault);
    exit(1);
}
 
int ParseData(char * temp)                      /*Parses string and updates request information structure if necessary*/
{


    char *last;
    int size;


    if ( fl == 1 )                           /* The first header line is the one where we can get the method type and resource from..  */
	{
	  if ( !strncmp(temp, "GET ", 4) )               /* Gets the request method */
	  {
	    method = 0;            /*0 is used to represent get method*/
	    temp = temp + 4;                                   
	  }
	
	while ( *temp && isspace(*temp) )             
	{  
      temp++;
    }


	last = strchr(temp, ' ');                     

	if ( last == NULL )
	  {
      size = strlen(temp);
	  }
	else
	  {
      size = last - temp;
      }
	
	if ( size == 0 ) 
	{
	    status = 400;                           /*if its a bad request , then status 400.. */
	    return -1;
	}

	file = calloc(size + 1, sizeof(char));                   /*  store it in the resource */
	strncpy(file, temp, size);

	fl = 0;
	return 0;
    }
    return 0;
}

int getrequestheader(int net)                /* Gets the request headers */ 
{
    char   temp[1000] ;
		
 	    int n = recvfrom(net,temp,1024,0,(struct sockaddr *)&from,&fromlen);                            /* Read the input */
        if (n < 0) 
		error("ERROR reading from socket");
		
		printf(temp);
	   
        removets(temp);                                /*  Removes trailing spaces from a string  */
	    
		ParseData(temp);                             /*Parses string and updates request information structure if necessary*/
		 
    return 0;
}

int removets(char * buff)                            /*  Removes trailing spaces from a string  */
{
    int n = 0;
	for ( n = strlen(buff) - 1; n >= 0; n--)
	{
		if(!isalnum(buff[n]))                 /* Replaces the non alpha-nummeric characters (white spaces) with a "\0", which indicates the end of the string*/
			buff[n]='\0';
		else
			break;
	}
    return 0;
}

int ack(int net, int x)        /* Returns resource if there is no error*/
{
    char a; int  b;

    while ( (b = read(x, &a, 1)) )                                   
	{
	  if ( b < 0 )
	   {
       printf("File encountered read error");
	   exit(1);
	   }
	  if ( write(net, &a, 1) < 1 )
	   {  
       printf("File encountered write error");
	   exit(1);
       }	
	}
    return 0;
}

int review() 
{  
	char file_dir[100] = "/u/vensamba/bin";                /*Root directory under SILO where the HTML file is stored*/

    strcat(file_dir, file);                     /*Here we combine the resource name with the server root. Eg:/u/vensamba/bin/hello.html*/

    int x = open(file_dir, O_RDONLY);  /*Returns the opened file*/
	return x;
}

int output(int net)      /*Outputs HTTP response headers*/
{
    
    char temp[500];
    sprintf(temp, "HTTP/1.1 %d OK\r\n", status);                                        /* output the http status */
	char str1[]=("Server: PGWebServ v0.1\r \n Content-Type: text/html\r \n");
	strcat(temp,str1);
	int x= strlen(temp);
    sendto(net, temp, x,0,(const struct sockaddr *)&server,length);	                  /* Write the headers into the socket */
    return 0; 
}


int InputData(int net) 
{


    int resource = 0;

    if ( getrequestheader(net) < 0 )                      /*  Get HTTP request  */
	return -1;
    output(net);
    if ( (resource = review()) < 0 )          /* Check whether resource exists and updates the status code */
	{
	  output(net);
	  return 1;
	  
	}
	printf("Status is: %d\n",status);
    
	                            /*Outputs HTTP response headers  */

    /*  Service the HTTP request  */

   if ( status == 200 ) 
	{
	if ( ack(net, resource) )                  /* returns the resource */
	    error("Error returning the resource");
     }


    if ( resource > 0 )
	if ( close(resource) < 0 )
	{
	  error("Resource could not be closed"); 
	}
    return 0;
}


int main(int argc, char **argv)
{
   int sock, newsock, length, n;
   struct sockaddr_in server;
   struct sockaddr_in from;
   char buf[1024];
    
	if (argc != 2) 
	 {                                                  /* Need 2 arguments: file name and port no.. */
         printf("Please provide a port number: \n");
         exit(1);
     }
     
	 
   sock=socket(AF_INET, SOCK_DGRAM, 0);
   if (sock < 0) error("Opening socket");
   length = sizeof(server);
   bzero(&server,length);
   server.sin_family=AF_INET;
   server.sin_addr.s_addr=INADDR_ANY;
   server.sin_port=htons(atoi(argv[1]));
	 
	 if (bind(sock,(struct sockaddr *)&server,length)<0) 
       error("binding");
   fromlen = sizeof(struct sockaddr_in);
   while (1) {
       
	   newsock = recvfrom(sock,buf,1024,0,(struct sockaddr *)&from,&fromlen);
       if (n < 0) error("recvfrom");
	   
	   InputData(newsock);
	    
   }
   return 0;
   
 }
