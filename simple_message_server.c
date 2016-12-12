
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
 * Last Modified: $Author: Gerhard $
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

#define BACKLOG 6

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
	//TCP SERVER - VALUES ------------------------------------START
	cpFilename = argv[0];
    
    int save_errno = 0; //*value for saving errno bevor it will be overritten*/
    
    int sfd, cfd;
    pid_t childpid;
    
    int optval; /* flag value for setsockopt */
    struct sockaddr_in peer_addr; /* server's addr */
    
    
    struct sockaddr_in clientaddr; /* client addr */
    struct hostent *hostp; /* client host info */
    
    socklen_t clientlen; /* byte size of client's address */
    char *hostaddrp; /* dotted decimal host addr string */
    
    
    //TCP SERVER - VALUES ------------------------------------END
    
    
    
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
    setsockopt(sfd, SOL_SOCKET, SO_REUSEADDR,(const void *)&optval , sizeof(int));

    
    
    
    /*
     * build the server's Internet address
     */
    bzero((char *) &peer_addr, sizeof(peer_addr)); //write zeroes to a byte string
    
    /* this is an Internet address */
    peer_addr.sin_family = AF_INET;
    
    /* let the system figure out our IP address */
    peer_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    
    //Convert char Array to INT
    int int_cpPort = atoi( cpPort );
    
    /* this is the port we will listen on */
    peer_addr.sin_port = htons((unsigned short)int_cpPort);

    
    
    
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

    /*CHECK IF BACKLOG IS BIGGER THAN SOMAXCONN -> IF SO, THAN EXIT -> IS NOT ALLOWED*/
    if (BACKLOG > SOMAXCONN) {
        fprintf(stderr,"BACKLOCK VALUE = %d IS BIGGER THAN ALLOWED = %d",BACKLOG,SOMAXCONN);
        close(sfd);
        exit(1);
    }
    
    
	// listen
    if (listen(sfd,BACKLOG)==-1) {
	   fprintf(stderr,"Could not start listener");
	   close(sfd);    
	   exit(1);
	}       

    
    
    /*
     * main loop: wait for a connection request
     *
     */

    
    clientlen = sizeof(struct sockaddr_in);
    
	// WHILE LOOP - START
	while (1) {
		
        
        /*
         * accept: wait for a connection request
         */
        
        //reset errno
        errno = 0;
        
        cfd = accept(sfd, (struct sockaddr *) &clientaddr, &clientlen);
        
        if (cfd < 0) {
            if(errno == EWOULDBLOCK || errno == EAGAIN) { /*The socket is marked nonblocking and no connections 
                                                           are present to be accepted. */
                continue;
            
            } else {
                fprintf(stderr,"Could not accept connection\n");
                exit(1);
            }
        }       
        /**------------------------------------------------------------*/
        
        
        /*
         * gethostbyaddr: determine who sent the message
         */
        hostp = gethostbyaddr((const char *)&clientaddr.sin_addr.s_addr, sizeof(clientaddr.sin_addr.s_addr), AF_INET);
        
            if (hostp == NULL) fprintf(stderr,"ERROR on gethostbyaddr");

        hostaddrp = inet_ntoa(clientaddr.sin_addr);
        
            if (hostaddrp == NULL) fprintf(stderr,"ERROR on inet_ntoa\n");

        printf("server established connection with %s (%s)\n",hostp->h_name, hostaddrp);
        
        /**------------------------------------------------------------*/

        
        //reset errno
        errno = 0;
        
		// fork
		childpid = fork();
        
        //store errno
        save_errno =errno;
        
        
		// CHILD process after fork
		if(childpid == (pid_t) 0) {
			
			// close
            close(sfd);
            
            
            if((dup2(cfd, 0) == -1)) {
                fprintf(stderr,"Could not read -dup2\n");
                close(cfd);
                exit(1);
            }
            
            if((dup2(cfd, 1) == -1)) {
                fprintf(stderr,"Could not write -dup2\n");
                close(cfd);
                exit(1);
            }

            //when everything is ok -> exec server logic
            close(cfd);
            (void) execl("/usr/local/bin/simple_message_server_logic", "simple_message_server_logic", (char*) NULL);
            exit(1);
            
		}
		// PARENT process after fork
		else if (childpid > (pid_t) 0) {
			//close child socket because parent is in the house
            close(cfd);
	
		}
		else {
			// FORK FAILED
			
            if (save_errno == EAGAIN) { /*The system-imposed limit on the total number 
                                         of processes under execution would be exceeded..
                                         --> TRY AGAIN -> MAY ME NEXT LOOP IS BETTER*/
                continue;
            }
            
           //ENOMEM: There is insufficient swap space for the new process
            close (sfd); //Close Parent Socket
            close (cfd); //Close Child Socket
            
            exit(1);
		}	

        
        
	//WHILE-LOOP-END
	}
    
	//close(cfd);
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
    //reset errno for checking fprintf()
    errno = 0;
    if (fprintf(stream,
            "\n usage: %s options\n"
            "options:\n"
            "        -p, --port <port>       port of the server [0 to 65535]\n"
            "        -h, --help\n", message) > 0) {
        errcode = errno; /*When fprintf fails, the new exit value is the errno value from the failed fprintf()*/
    }
   
    exit(errcode);
}

