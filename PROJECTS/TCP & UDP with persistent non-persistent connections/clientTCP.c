/*TCP: Client for persistent and non-persistent*/

#include <stdio.h>                                                      
#include <string.h>
#include <sys/types.h>
#include <stdlib.h>
#include <unistd.h>
#include <netdb.h>
#include <errno.h> 
#include <netinet/in.h>
#include <sys/socket.h>



char CT[] = "non-persistent";                                /* Globally declaring connection type as non-persistent */
char END[]={'*'};

int removeslashn(char * y)                         /* When I read line by line, it even includes \n character. So getting rid of that using this function */
{                              
  int n = strlen(y) - 1;

  while ((!isalnum(y[n])) && (n >= 0))	             /* !isalnum checks for non-alpha numeric characters */
  y[n--] = '\0';
  return 0;
}

int writeheader(int res, char *x, int i)                             /*  Write a line to a socket  */
{
    int r,w;
    char *b;

    b = x; r = i;
    
    while ( r > 0 ) 
	{
	  w = write(res, b, r);
	  r  -= w;
	  b += w;
    }

    return i;
}



int readheaders(int sock, void *z)                                                               /*  Read a line from a socket  */
{
    int i, count;
    char    req, *w, *ret;
    i = 0;
    w = z;
    
	  while (((count = read(sock, &req, 1)) == 1) && (req != '\n'))
	  {
	    *w++ = req;
		i++;
	  }

    *w = 0;
	
	if(((!i) && (req != '\n'))) 
		return 1;
	else 
		return 0;
}

void error(const char *msg)                                                          /*Dislays the error message*/
{
    perror(msg);
    exit(0);
}

int main(int argc, char *argv[])                       
{
    int sock, port;
    struct sockaddr_in serv_addr;
    struct hostent *server;

    char buffer[500];
    char temp[500];
	char fileName[500];
	char httpVersion[]="1.0";
	
    if (argc < 3) {
       fprintf(stderr,"usage %s hostname port\n", argv[0]);
       exit(0);
    }
	
	if (argc < 4) 
	{
		printf("Enter the connection type: persistent (p) or non-persistent (np))\n");
		exit(1);
	}
	strcpy(CT,argv[3]);
	
	if (!(strcmp(argv[3],"p")))                                                         /*If its persistent set to 1.1, else 1.0 */
		strcpy(httpVersion,"1.1");
		else 
	strcpy(httpVersion,"1.0");		
    port = atoi(argv[2]);                                                                  /*Converts the port no from string to integer*/
    sock = socket(AF_INET, SOCK_STREAM, 0);                                               /*creates a new socket*/
    
	if (sock < 0) 
        error("ERROR opening socket");
    server = gethostbyname(argv[1]);                                                        /*Gets host name*/
    
	if (server == NULL) {
        fprintf(stderr,"ERROR, no such host\n");
        exit(0);
    }
    bzero((char *) &serv_addr, sizeof(serv_addr));                                           /*clearing the buffer*/                            
    serv_addr.sin_family = AF_INET;
    bcopy((char *)server->h_addr, 
         (char *)&serv_addr.sin_addr.s_addr,
         server->h_length);
    serv_addr.sin_port = htons(port);
    
	if (connect(sock,(struct sockaddr *) &serv_addr,sizeof(serv_addr)) < 0)                          /*Connect the socket to the address of the server*/
        error("ERROR connecting");
	 
	/*If version is 1.0 its non-persistent. This works sometimes with a few ports, but sometimes with a different port numbers it does't work. Not sure what the error is..*/
	
	if(!strcmp(httpVersion,"1.0"))  	
    {
	sprintf(buffer, "GET /%s HTTP/%s\r\n", argv[4],httpVersion);
    writeheader(sock, buffer, strlen(buffer));
	bzero(buffer,500);                                                                      /*clear the buffer*/
	writeheader(sock, "Connection: close\r\n", 24);
	sprintf(buffer, "Host: silo.soic.indiana.edu:%d\r\n", port);
	writeheader(sock, buffer, strlen(buffer));
	writeheader(sock, "User-Agent: Chrome/15.0\r\n", 27);
	writeheader(sock, "Accept-Language: en-US\r\n", 24);                              /*Writing the header request to the server*/
    writeheader(sock, "\r\n", 2);

	int i=0,j,res;
	
	while(1)
	{
	bzero(fileName,500);                                                                         
		if (readheaders(sock, fileName)) break;                                        /*reading the response from the server*/
		if(strstr(temp, END)!=NULL) break;
		printf("%s\n",fileName);
	} 
    printf("\n");
    close(sock);
	}
	
	else if(!strcmp(httpVersion,"1.1"))                                                       /*If version is 1.1 its persistent*/
	{	
      

                                                    
   FILE *file = fopen ( argv[4], "r" );                   /*input file*/
   if ( file != NULL )
   {
        char line [ 50 ];                                              


        while ( fgets ( line, sizeof line, file ) != NULL )                                       /* reads line by line*/
       {
			printf(line,stdout); 
			
		bzero(buffer,500);
		removeslashn(line);                          /* When I read line by line, it even includes \n character. So getting rid of that using this function*/

		
			char buffer11[250];
		sprintf(buffer, "GET /%s HTTP/%s\r\n", line,httpVersion);
		printf(buffer);
		sprintf(buffer11, "Host: silo.soic.indiana.edu:%d\r\n", port);
		char buffer12[200]= ("Connection: keep-alive\r\n");
		char buffer13[150]= ("User-Agent: Chrome/15.0\r\n");
		char buffer14[80]= ("Accept-Language: en-US\r\n");
		char buffer15[30]= ("\r\n");
		strcat(buffer14,buffer15);strcat(buffer13,buffer14);
		strcat(buffer12,buffer13);strcat(buffer11,buffer12);strcat(buffer,buffer11);
        int y = strlen(buffer);
        write(sock,buffer,y);
		
			
			while(1)
				{
					bzero(temp,500);
					if (readheaders(sock, temp)) break;                    /*reads the output response from the server*/
					if(strstr(temp, END)!=NULL) break;                /* The file will quit only if there is "*" in the end.. */
					printf("%s\n",temp);                                           /*displays the output response*/
				}
				    printf("\n");
	}
	fclose ( file ); 
    close(sock);	                                                    /*close the file*/
	  }
	}
	 close(sock);                                                       /*close the socket*/
    return 0;
}
