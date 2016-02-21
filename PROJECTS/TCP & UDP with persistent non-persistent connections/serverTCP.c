/*TCP: Server for persistent and non-persistent*/

#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <errno.h>
#include <sys/types.h>
#include <fcntl.h>
#include <sys/socket.h>
#include <netinet/in.h>

/*Global variables*/
                                         
int fl = 1, status = 200, method = 0;            /*f1 repesents the first header line*/ /*0 means method is get*/
char CS[] = "0";                                                 /*0 represents connection status as keep alive*/
char *file;
                                                             

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
		
 	    int n = read(net,temp,1000);                            /* Read the input */
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
	write(net,temp,x);                                                                              /* Write the headers into the socket */
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
     char buff[256];
	 int sock, newsock, port;
	 pid_t  pid;
     socklen_t clen;
     struct sockaddr_in serv_addr, cli_addr;
    
	if (argc != 2) 
	 {                                                  /* Need 2 arguments: file name and port no.. */
         printf("Please provide a port number: \n");
         exit(1);
     }
     sock = socket(AF_INET, SOCK_STREAM, 0);             /*creates a new socket*/
     
	 if (sock < 0)
     {
      error("Could not open the socket..");
     }
	 
	 bzero((char *) &serv_addr, sizeof(serv_addr));
     port = atoi(argv[1]);                               /* Converts the port no. from character to integer */
     serv_addr.sin_family = AF_INET;
     serv_addr.sin_addr.s_addr = INADDR_ANY;
     serv_addr.sin_port = htons(port);
     
	 if (bind(sock, (struct sockaddr *) &serv_addr,           /*it binds the socket to an address*/
        sizeof(serv_addr)) < 0)
        error("Binding error");
     listen(sock,5);                                           /*can listen to a maximum of 5 connections*/
     clen = sizeof(cli_addr);
     newsock = accept(sock,(struct sockaddr *) &cli_addr, &clen);            
     
	 if (newsock < 0)
	 {
		error("Accepting error"); 
	 } 
     bzero(buff,256);
     do
	 {
		fl =1;
	    if (InputData(newsock)) break;                         /*The request from the client is sent here. It breaks if file not found*/
	 }
	 while((!strcmp(CS,"0")));                     /*0 represents connection status as keep alive. So it executes a persistent execution if its 0 */
	
	 close(newsock);
     close(sock);
     return 0;
}
