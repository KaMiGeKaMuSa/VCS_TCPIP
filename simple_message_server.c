
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
    //int iSocketFD = NULL;
    cpFilename = argv[0];
    
    /* function to parse parameter provided by Thomas M. Galla, Christian Fibich*/
    smc_parsecommandline(argc, argv, &usage, &cpPort);
    
    
    //openSocket(&iSocketFD);
    
    //sendRequest(&iSocketFD);
    
    //readResponse(&iSocketFD);
    
    //close(iSocketFD);
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

