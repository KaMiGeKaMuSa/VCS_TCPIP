
/**
 * @file simple_message_server.c
 * TCP/IP Server-Client project
 *
 * @author Karin Kalman <karin.kalman@technikum-wien.at>
 * @author Michael Mueller <michael.mueller@technikum-wien.at>
 * @author Gerhard Sabeditsch <gerhard.sabeditsch@technikum-wien.at>
 * @date 2016/12/10
 *
 * @version $Revision: 1 $
 *
 *
 * URL: $HeadURL$
 *
 * Last Modified: $Author: Karin $
 */


/***** COMMENTS: *********
 socket (AF_INET, SOCK_STREAM, 0)
 bind
 listen
	accept
	fork
 dup2 für stdin und stdout
	exec (businesslogic ausführen)
 close für stdin und stdout
	close
 
 getopt für port
 bind bsp in manpage von ip(7)
 struct sockaddr_in:
	- sa_family = AF_INET;
	- sin_port = htans(portnummer)
	- sin_addr = INADDR_ANY
	
 setsockopt(SOL_REUSEADDR,1)
 
 */



/**
 * -------------------------------------------------------------- includes --
 */
#include "simple_message_server_commandline_handling.h"
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <string.h>
#include <unistd.h>
//new included
#include <netinet/in.h>
#include <arpa/inet.h>
/**
 * -------------------------------------------------------------- defines --
 */



/**
 * -------------------------------------------------------------- global variables --
 */
const char *cpPort, *cpFilename;

/**
 * --------------------------------------------------- function prototypes --
 */
void usage(FILE * stream, const char * message, int exitcode);


/**
 * ------------------------------------------------------------- main --
 */
int main(int argc, const char* argv[])
{
	//SERVER - VALUES ------------------------------------START
	cpFilename = argv[0];

    int sfd, cfd;
    pid_t childpid;
    
    int optval; /* flag value for setsockopt */
    struct sockaddr_in peer_addr; /* server's addr */
    
    
    struct sockaddr_in clientaddr; /* client addr */
    struct hostent *hostp; /* client host info */
    
    socklen_t clientlen; /* byte size of client's address */
    char *hostaddrp; /* dotted decimal host addr string */
    
    
    //SERVER - VALUES ------------------------------------END
    
    
    
	/* function to parse parameter provided by Thomas M. Galla, Christian Fibich*/
	smc_parsecommandline(argc, argv, &usage, &cpPort);
    
    
    /*
     * socket: create the parent socket
     */
    
	//socket
	sfd = socket(AF_INET, SOCK_STREAM, 0);
	if (sfd == -1)
	{
	   fprintf(stderr,"Could not create socket");
	   exit(1);
	}		   
	
    
    /* setsockopt: Handy debugging trick that lets
     * us retun the server immediately after we kill it;
     * otherwise we have to wait about 20 secs.
     * Eliminates "ERROR on binding: Address already in use" error.
     */
    optval = 1;
    setsockopt(sfd, SOL_SOCKET, SO_REUSEADDR,
               (const void *)&optval , sizeof(int));

    
    
    
    /*
     * build the server's Internet address
     */
    bzero((char *) &peer_addr, sizeof(peer_addr));
    
    /* this is an Internet address */
    peer_addr.sin_family = AF_INET;
    
    /* let the system figure out our IP address */
    peer_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    
    /* this is the port we will listen on */
    peer_addr.sin_port = htons((unsigned short)cpPort);

    
    
    
    /*
     * bind: associate the parent socket with a port
     */
    
    // bind
    if (bind(sfd, (struct sockaddr *) &peer_addr, sizeof(peer_addr)) < 0) {
        fprintf(stderr,"Could not bind socket");
        close(sfd);
        exit(1);
    }
    
    
    
    /*
     * listen: make this socket ready to accept connection requests
     */

	// listen
	//if (listen(sfd,LISTEN_BACKLOG)==-1)
    if (listen(sfd,SOMAXCONN)==-1)
    {
	   fprintf(stderr,"Could not start listener");
	   close(sfd);    
	   exit(1);
	}       

    
    
    /*
     * main loop: wait for a connection request
     *
     */

    
    clientlen = sizeof(struct sockaddr_in);
    
	// loop
	for (;;) {
		
        
        /*
         * accept: wait for a connection request
         */
        cfd = accept(sfd, (struct sockaddr *) &clientaddr, &clientlen);
        
        if (cfd == -1)
        {
            fprintf(stderr,"Could not accept connection");
            //close(sfd);  ## Nur weil Accept bei einem Client fehl schlaegt, sollte man nicht raus fliegen, oder?
            //exit(1);
        }       
        
        
        
        /*
         * gethostbyaddr: determine who sent the message
         */
        hostp = gethostbyaddr((const char *)&clientaddr.sin_addr.s_addr, sizeof(clientaddr.sin_addr.s_addr), AF_INET);
        
            if (hostp == NULL) fprintf(stderr,"ERROR on gethostbyaddr");

        hostaddrp = inet_ntoa(clientaddr.sin_addr);
        
            if (hostaddrp == NULL) fprintf(stderr,"ERROR on inet_ntoa\n");

        printf("server established connection with %s (%s)\n",hostp->h_name, hostaddrp);

        
        
        
		// fork
		childpid = fork();
	
		// Child process after fork
		if(childpid == (pid_t) 0)
		{
			// exec 

			// close
	
		}
		// Parent process after fork
		else if (childpid > (pid_t) 0)
		{
			// dup2
			
			// close
	
		}
		else 
		{
			// fork failed
			exit(1);
		}	

	
	}
			   

    return 0;
}



/**
 * \brief function needed as error message in smc_parsecommandline
 *
 * \param stream - stream where error message gets printed
 * \param message - error message print before exiting
 * \param errcode - int number which is used at exit
 */
void usage(FILE * stream, const char * message, int errcode)
{
    fprintf(stream, message);
    exit(errcode);
}

