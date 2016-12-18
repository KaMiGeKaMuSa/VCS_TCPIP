
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
int save_errno;

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
    
    save_errno = 0; //*value for saving errno bevor it will be overritten*/
    
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
	if (sfd == -1) {
	  
        //RESET save_errno
        save_errno = 0;
        
        //MAIN ERROR MESSAGE
        if(fprintf(stderr,"%s - %s: %s\n", cpFilename, "socket()", "Could not create socket") < 0) save_errno= errno;
        
        
        //EXIT LOGIC
        if (save_errno != 0) exit (save_errno); //Wenn save_errno ungleich 0, dann exit mit save_errno -> andernfalls mit normalen exit fehler
        exit(1);


	}		   
	
    
    /* setsockopt:
     * retun to the server immediately after we kill it -> otherwise -> wait about 20 secs.
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
    
    //Convert char Array into INT
    int int_cpPort = atoi( cpPort );
    
    /* this is the port we will listen on */
    peer_addr.sin_port = htons((unsigned short)int_cpPort);

    
    
    
    /*
     * bind: associate the parent socket with a port
     */
    
    // bind
    if (bind(sfd, (struct sockaddr *) &peer_addr, sizeof(peer_addr)) < 0) {
        
        //RESET save_errno
        save_errno = 0;
        
        //MAIN ERROR MESSAGE
        if(fprintf(stderr,"%s - %s: %s\n", cpFilename, "socket(),bind()", "Could not bind socket") < 0) save_errno= errno;
        
        
        //CLOSE PARENT SOCKET
        if (close(sfd) < 0 ) {
            if(fprintf(stderr,"%s - %s: %s\n", cpFilename, "socket(),bind()-close()", "Could not bind socket and could not close socket") < 0) save_errno= errno;
        }
        
        //EXIT LOGIC
        if (save_errno != 0) exit (save_errno); //Wenn save_errno ungleich 0, dann exit mit save_errno -> andernfalls mit normalen exit fehler
        exit(1);
        
    }
    
    
    
    /*
     * listen: make this socket ready to accept connection requests
     */

    /*CHECK IF BACKLOG IS BIGGER THAN SOMAXCONN -> IF SO, THAN EXIT -> IS NOT ALLOWED*/
    if (BACKLOG > SOMAXCONN) {
    
        //RESET save_errno
        save_errno = 0;
        
        //MAIN ERROR MESSAGE
        if(fprintf(stderr,"%s - %s: %s\n", cpFilename, "socket(),BACKLOG_CHECK", "BACKLOG Value BACKLOG is bigger than allowed (SOMAXCONN)") < 0) save_errno= errno;
        
        //CLOSE PARENT SOCKET
        if (close(sfd) < 0 ) {
            if(fprintf(stderr,"%s - %s: %s\n", cpFilename, "socket(),BACKLOG_CHECK-close()", "BACKLOG VALULE is bigger than allowed and could not close socket") < 0) save_errno= errno;
        }
      
        //EXIT LOGIC
        if (save_errno != 0) exit (save_errno); //Wenn save_errno ungleich 0, dann exit mit save_errno -> andernfalls mit normalen exit fehler
        exit(1);
    
    }
    
    
	// listen
    if (listen(sfd,BACKLOG)==-1) {
        
        //RESET save_errno
        save_errno = 0;
        
        //MAIN ERROR MESSAGE
        if(fprintf(stderr,"%s - %s: %s\n", cpFilename, "socket(),listen()", "Could not start listener") < 0) save_errno= errno;
        
        
        //CLOSE PARENT SOCKET
        if (close(sfd) < 0 ) {
            if(fprintf(stderr,"%s - %s: %s\n", cpFilename, "socket(),listen()-close()", "Could not start listener and could not close socket") < 0) save_errno= errno;
        }
        
        //EXIT LOGIC
        if (save_errno != 0) exit (save_errno); //Wenn save_errno ungleich 0, dann exit mit save_errno -> andernfalls mit normalen exit fehler
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
                continue; //try again
            
            } else {
                
                
                //RESET save_errno
                save_errno = 0;
                
                //MAIN ERROR MESSAGE
                if(fprintf(stderr,"%s - %s: %s\n", cpFilename, "socket(),accept()", "Could not accept connection") < 0) save_errno= errno;
                
                //CLOSE PARENT SOCKET
                if (close(sfd) < 0 ) {
                    if(fprintf(stderr,"%s - %s: %s\n", cpFilename, "socket(),accept()-close()", "Could not start listener and could not close socket") < 0) save_errno= errno;
                }
                
                //EXIT LOGIC
                if (save_errno != 0) exit (save_errno);
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
        
        
        // IF FORK FAILED
        if (errno == EAGAIN) { /*The system-imposed limit on the total number
                                     of processes under execution would be exceeded..
                                     --> TRY AGAIN -> MAYBE NEXT LOOP IS BETTER*/
            continue;
        }
        
        if (errno == ENOMEM){ //ENOMEM: There is insufficient swap space for the new process
            
            //RESET save_errno
            save_errno = 0;

            //CLOSE PARENT SOCKET
            if (close(sfd) < 0 ) {
                if(fprintf(stderr,"%s - %s: %s\n", cpFilename, "PARENT-fork()-close()", "Could not close PARENT socket") < 0) save_errno= errno;
            }

            //CLOSE CHILD SOCKET
            if (close(cfd) < 0 ) {
                if(fprintf(stderr,"%s - %s: %s\n", cpFilename, "CHILD-fork()-close()", "Could not close CHILD socket") < 0) save_errno= errno;
            }

            //EXIT LOGIC
            if (save_errno != 0) exit (save_errno);
            exit(1);
 
            
        }
        

        
		// CHILD process after fork
		if(childpid == (pid_t) 0) {
			
            //RESET save_errno
            save_errno = 0;

            
            //CLOSE PARENT SOCKET
            if (close(sfd) < 0 ) {
                if(fprintf(stderr,"%s - %s: %s\n", cpFilename, "PARENT-fork()-close()", "Could not close PARENT socket") < 0) save_errno= errno;
            
                //EXIT LOGIC
                if (save_errno != 0) exit (save_errno);
                exit (1);
            }
            
            
            if((dup2(cfd, 0) == -1)) {
                
                //RESET save_errno
                save_errno = 0;

                //MAIN ERROR MESSAGE
                if(fprintf(stderr,"%s - %s: %s\n", cpFilename, "CHILD-dup2()", "Could not read -dup2") < 0) save_errno= errno;

                //CLOSE CHILD SOCKET
                if (close(cfd) < 0 ) {
                    if(fprintf(stderr,"%s - %s: %s\n", cpFilename, "CHILD-dup2()-close()", "Could not read -dup2 and could not close socket") < 0) save_errno= errno;
                }
    
                //EXIT LOGIC
                if (save_errno != 0) exit (save_errno);
                exit(1);
                
                
            }
            
            if((dup2(cfd, 1) == -1)) {
                
                //RESET save_errno
                save_errno = 0;
                
                //MAIN ERROR MESSAGE
                if(fprintf(stderr,"%s - %s: %s\n", cpFilename, "CHILD-dup2()", "Could not write -dup2") < 0) save_errno= errno;
                
                //CLOSE CHILD SOCKET
                if (close(cfd) < 0 ) {
                    if(fprintf(stderr,"%s - %s: %s\n", cpFilename, "CHILD-dup2()-close()", "Could not write -dup2 and could not close socket") < 0) save_errno= errno;
                }
                
                //EXIT LOGIC
                if (save_errno != 0) exit (save_errno);
                exit(1);
            
            }

            //when everything is ok -> exec server logic
           
            //RESET save_errno
            save_errno = 0;
            
            //CLOSE CHILD SOCKET
            if (close(cfd) < 0 ) {
                if(fprintf(stderr,"%s - %s: %s\n", cpFilename, "CHILD-fork()-close()", "Could not close CHILD socket") < 0) save_errno= errno;
                
                //EXIT LOGIC
                if (save_errno != 0) exit (save_errno);
                exit(1);
            }

            
            if( execl("/usr/local/bin/simple_message_server_logic", "simple_message_server_logic", (char*) NULL)< 0) {
                //RESET save_errno
                save_errno = 0;

                if(fprintf(stderr,"%s - %s: %s\n", cpFilename, "CHILD-fork()-simple_message_server_logic()", "Could not START simple_message_server_logic properly") < 0) save_errno= errno;
                
                //EXIT LOGIC
                if (save_errno != 0) exit (save_errno);
                exit(1);
                
            }
            
            exit(1);
            
            
		}
		// PARENT process after fork
		else if (childpid > (pid_t) 0) {
			
            //close child socket because parent is in the house
            
            //RESET save_errno
            save_errno = 0;
            
            //CLOSE CHILD SOCKET
            if (close(cfd) < 0 ) {
                if(fprintf(stderr,"%s - %s: %s\n", cpFilename, "PARENT-fork()-close()", "Could not close CHILD socket") < 0) save_errno= errno;
            
                //EXIT LOGIC
                if (save_errno != 0) exit (save_errno);
                exit(1);
            }
           
            
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
            "        -h, --help\n", message) < 0) {
        errcode = errno; /*When fprintf fails, the new exit value is the errno value from the failed fprintf()*/
    }
   
    exit(errcode);
}

